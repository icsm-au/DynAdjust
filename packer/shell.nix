let
  defaultPkgs = import <nixpkgs> {};
  pinnedPkgs = import (defaultPkgs.fetchFromGitHub {
    owner = "NixOS";
    repo = "nixpkgs-channels";
    rev = "6141939d6e0a77c84905efd560c03c3032164ef1"; # 7 November 2018
    sha256 = "1nz2z71qvjna8ki5jq4kl6pnl716hj66a0gs49l18q24pj2kbjwh";
  }) {};
in
  { nixpkgs ? pinnedPkgs }:

  let
    pkgs = if nixpkgs == null then defaultPkgs else pinnedPkgs;

    devEnv = with pkgs; buildEnv {
      name = "devEnv";
      paths = [
        awscli
        cacert
        jq
        terraform_0_11
        packer
        pythonPackages.credstash
      ];
    };
  in
  pkgs.runCommand "setupEnv" {
    buildInputs = [
      devEnv
    ];
    shellHook = ''
    export SSL_CERT_FILE="${pkgs.cacert}/etc/ssl/certs/ca-bundle.crt"

    # workaround due to some bug in nixpkgs
    function credstash() {
    "${pkgs.pythonPackages.credstash}/bin/credstash.py" $@
    }
    export -f credstash

    export LS_COLORS=$LS_COLORS:'di=1;4;36'
    parse_git_branch() {
    git branch 2> /dev/null | sed -e '/^[^*]/d' -e 's/* \(.*\)/\1/'
    }
    export PS1="\n\[\033[1;32m\]\e[35m\e[4m$(parse_git_branch)\e[24m\e[0m \w\$ \[\033[0m\]"
    '';
  } ""

