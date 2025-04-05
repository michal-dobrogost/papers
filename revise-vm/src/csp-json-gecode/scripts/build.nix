with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "env";
  nativeBuildInputs = [ cmake gecode ]; # build time
  buildInputs = []; # runtime
}
