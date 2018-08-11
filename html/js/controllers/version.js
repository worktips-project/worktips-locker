thinwalletCtrls.controller('VersionCtrl', function ($scope, $http, ApiCalls) {

    $scope.last_git_commit_date = "";
    $scope.last_git_commit_hash = "";
    $scope.loki_version_full = "";

    var version = ApiCalls.getVersion()
        .then(function(response) {

            var last_git_commit_date = response.data.last_git_commit_date;
            var last_git_commit_hash = response.data.last_git_commit_hash;
            var git_branch_name      = response.data.git_branch_name;
            var loki_version_full    = response.data.loki_version_full;

            // on the backend, api version is in uint32 format.
            var api_major = response.data.api >> 16;
            var api_minor = response.data.api & 0xffff;

            $scope.version = "Loki Locker version (api): "
                + git_branch_name + "-" + last_git_commit_date + "-" + last_git_commit_hash
                + " (" + api_major + "." + api_minor + ")"
                + "  | Loki version: " + loki_version_full
                + "  | Blockchain height: " + response.data.blockchain_height

        }, function(response) {
            $scope.version = "Error: Can't connect to the backend! Maybe it is down."
        });
});