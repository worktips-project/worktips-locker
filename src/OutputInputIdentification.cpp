//
// Created by mwo on 13/02/17.
//

#include "OutputInputIdentification.h"


namespace xmreg
{

OutputInputIdentification::OutputInputIdentification(
    const address_parse_info* _a,
    const secret_key* _v,
    const transaction* _tx,
    crypto::hash const& _tx_hash,
    bool is_coinbase,
    std::shared_ptr<CurrentBlockchainStatus> _current_bc_status)
    : current_bc_status {_current_bc_status}
{
    matched_pub_key_index = -1;
    address_info = _a;
    viewkey = _v;
    tx = _tx;
    tx_pub_key_str = "";

    tx_pub_keys  = xmreg::get_tx_pub_keys_from_received_outs(*tx);

    tx_is_coinbase = is_coinbase;
    tx_hash = _tx_hash;

    is_rct = (tx->version >= 2);

    if (is_rct)
    {
        rct_type = tx->rct_signatures.type;
    }


    for (const auto& tx_pub_key : tx_pub_keys)
    {
        derivations.resize(derivations.size() + 1);
        if (!generate_key_derivation(tx_pub_key, *viewkey, derivations.back().first))
        {
            OMERROR << "Cant get derived key for: "  << "\n"
                 << "pub_tx_key: " << pod_to_hex(tx_pub_key) << " and "
                 << "prv_view_key" << viewkey;;

            throw OutputInputIdentificationException(
                        "Cant get derived key for a tx");
        }
        derivations.back().second = tx_pub_key;
    }
}

uint64_t
OutputInputIdentification::get_mixin_no()
{
    if (mixin_no == 0 && !tx_is_coinbase)
        mixin_no = xmreg::get_mixin_no(*tx);

    return mixin_no;
}

void
OutputInputIdentification::identify_outputs()
{
    //          <public_key  , amount  , out idx, unlock time>
    std::vector<outputs_tuple> outputs = get_outputs_tuple(*tx);

    for (auto& out: outputs)
    {
        if (std::get<0>(out).type() != typeid(txout_to_key))
        {
            continue;
        }

        const txout_to_key& txout_k
            = boost::get<cryptonote::txout_to_key>(std::get<0>(out));
        uint64_t amount             = std::get<1>(out);
        uint64_t output_idx_in_tx   = std::get<2>(out);
        uint64_t unlock_time        = std::get<3>(out);

        // get the tx output public key
        // that normally would be generated for us,
        // if someone had sent us some xmr.

        size_t matched_index = 0;
        for (const auto& derivation : derivations)
        {
            matched_index++;
            public_key derived_output_pubkey;

            derive_public_key(derivation.first,
                    output_idx_in_tx,
                    address_info->address.m_spend_public_key,
                    derived_output_pubkey);

            // check if generated public key matches the current output's key
            if (txout_k.key != derived_output_pubkey) continue;

            matched_pub_key_index = matched_index - 1;

            // placeholder variable for ringct outputs info
            // that we need to save in database
            string rtc_outpk;
            string rtc_mask;
            string rtc_amount;

            // initialize with regular amount value
            // for ringct, except coinbase, it will be 0
            uint64_t rct_amount_val = amount;

            // cointbase txs have amounts in plain sight.
            // so use amount from ringct, only for non-coinbase txs
            if (!tx_is_coinbase)
            {
                bool r;

                // for ringct non-coinbase txs, these values are given
                // with txs.
                // coinbase ringctx dont have this information. we will provide
                // them only when needed, in get_unspent_outputs. So go there
                // to see how we deal with ringct coinbase txs when we spent
                // them
                // go to CurrentBlockchainStatus::construct_output_rct_field
                // to see how we deal with coinbase ringct that are used
                // as mixins
                rtc_outpk  = pod_to_hex(tx->rct_signatures
                        .outPk[output_idx_in_tx].mask);
                rtc_mask   = pod_to_hex(tx->rct_signatures
                        .ecdhInfo[output_idx_in_tx].mask);
                rtc_amount = pod_to_hex(tx->rct_signatures
                        .ecdhInfo[output_idx_in_tx].amount);

                rct::key mask =  tx->rct_signatures
                    .ecdhInfo[output_idx_in_tx].mask;

                r = decode_ringct(tx->rct_signatures,
                        derivation.second,
                        *viewkey,
                        output_idx_in_tx,
                        mask,
                        rct_amount_val);

                if (!r)
                {
                    OMERROR << "Cant decode ringCT!";
                    throw OutputInputIdentificationException(
                            "Cant decode ringCT!");
                }

                amount = rct_amount_val;

            } // if (!tx_is_coinbase)

            total_received += amount;

            identified_outputs.emplace_back(
                        output_info{
                                txout_k.key, derivation.second, amount, output_idx_in_tx,
                                rtc_outpk, rtc_mask, rtc_amount, unlock_time
                        });

        }
    } // for (const auto& out: outputs)

}


void
OutputInputIdentification::identify_inputs(
        unordered_map<public_key, uint64_t> const& known_outputs_keys)
{
    vector<txin_to_key> input_key_imgs = xmreg::get_key_images(*tx);

    size_t search_misses {0};

    // make timescale maps for mixins in input
    for (txin_to_key const& in_key: input_key_imgs)
    {
        // get absolute offsets of mixins
        std::vector<uint64_t> absolute_offsets
                = cryptonote::relative_output_offsets_to_absolute(
                        in_key.key_offsets);

        // get public keys of outputs used in the mixins that
        // match to the offests
        std::vector<cryptonote::output_data_t> mixin_outputs;

        if (!current_bc_status->get_output_keys(in_key.amount,
                                                absolute_offsets,
                                                mixin_outputs))
        {
            OMERROR << "Mixins key images not found";
            continue;
        }

        // indicates whether we found any matching mixin in the current input
        bool found_a_match {false};

        // for each found output public key check if its ours or not
        for (size_t count = 0; count < absolute_offsets.size(); ++count)
        {
            // get basic information about mixn's output
            cryptonote::output_data_t const& output_data
                    = mixin_outputs[count];

            //cout << " - output_public_key_str: "
            // << output_public_key_str;

            // before going to the mysql, check our known outputs cash
            // if the key exists. Its much faster than going to mysql
            // for this.

            auto it = known_outputs_keys.find(output_data.pubkey);

            if (it != known_outputs_keys.end())
            {                                                
                // this seems to be our mixin.
                // save it into identified_inputs vector

                identified_inputs.push_back(input_info {
                        pod_to_hex(in_key.k_image),
                        it->second, // amount
                        output_data.pubkey});

                //cout << "\n\n" << it->second << endl;

                found_a_match = true;
            }

        } // for (const cryptonote::output_data_t& output_data: outputs)

        if (found_a_match == false)
        {
            // if we didnt find any match, break of the look.
            // there is no reason to check remaining key images
            // as when we spent something, our outputs should be
            // in all inputs in a given txs. Thus, if a single input
            // is without our output, we can assume this tx does
            // not contain any of our spendings.

            // just to be sure before we break out of this loop,
            // do it only after two misses

            if (++search_misses > 2)
                break;
        }

    } //   for (txin_to_key const& in_key: input_key_imgs)

}


string const&
OutputInputIdentification::get_tx_hash_str()
{
    if (tx_hash_str.empty())
        tx_hash_str = pod_to_hex(tx_hash);

    return tx_hash_str;
}


string const&
OutputInputIdentification::get_tx_prefix_hash_str()
{
    if (tx_prefix_hash_str.empty())
    {
        tx_prefix_hash = get_transaction_prefix_hash(*tx);
        tx_prefix_hash_str = pod_to_hex(tx_prefix_hash);
    }

    return tx_prefix_hash_str;
}

// this function should no longer be required, but is kept to give the db
// one of the tx pub keys to associate with the transaction
string const&
OutputInputIdentification::get_tx_pub_key_str()
{
    if (tx_pub_key_str == "")
    {
        if (tx_pub_keys.size() > 0)
        {
            tx_pub_key_str = pod_to_hex(tx_pub_keys[0]);
        }
        else
        {
            tx_pub_key_str = "0000000000000000000000000000000000000000000000000000000000000000";
        }
    }
    return tx_pub_key_str;
}

}
