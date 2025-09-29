{
  pkgs ? import <nixpkgs> {},
}: pkgs.mkShell {
  packages = with pkgs; [
    gcc
    clang-tools
    gdb
  ];
}
