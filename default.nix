{ pkgs ? import <nixpkgs> {}
, useLLVMLibcxx ? false
, ... }:

with pkgs;

pkgs.llvmPackages.stdenv.mkDerivation rec {
  name = "xeus-cling";
  src = ./.;

  nativeBuildInputs = with pkgs; [
    cmake
    makeWrapper
  ];

  buildInputs = with pkgs; [
    xeus
    xeus-zmq
    cppzmq
    cling.unwrapped
    argparse
    xtl
    pugixml
    nlohmann_json
    openssl
    llvmPackages_9.libllvm
    llvmPackages_9.libclang
    libuuid
  ];

  # Runtime flags for the C++ standard library
  cxxFlags = if useLLVMLibcxx then [
    "-I" "${lib.getDev llvmPackages_9.libcxx}/include/c++/v1"
    "-L" "${llvmPackages_9.libcxx}/lib"
    "-l" "${llvmPackages_9.libcxx}/lib/libc++.so"
  ] else [
    "-I" "${gcc-unwrapped}/include/c++/${gcc-unwrapped.version}"
    "-I" "${gcc-unwrapped}/include/c++/${gcc-unwrapped.version}/x86_64-unknown-linux-gnu"
  ];

  flags = [
    "-nostdinc"
    "-nostdinc++"
    "-isystem" "${cling.unwrapped}/lib/clang/9.0.1/include"
  ] ++ cxxFlags ++ [
    "-isystem" "${lib.getDev stdenv.cc.libc}/include"
    "-isystem" "${cling.unwrapped}/include"
  ];

  fixupPhase = ''
    wrapProgram $out/bin/xcpp --add-flags "$flags"
  '';
}
