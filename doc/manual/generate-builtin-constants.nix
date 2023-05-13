let
  inherit (builtins) concatStringsSep attrNames;
  inherit (import ./utils.nix) optionalString;
in

builtinsInfo:
let
  showBuiltin = name:
    let
      inherit (builtinsInfo.${name}) doc type;
      type' = optionalString (type != null) " (${type})";
    in
    ''
      <dt id="builtin-constants-${name}">
        <a href="#builtin-constants-${name}"><code>${name}</code>${type'}</a>
      </dt>
      <dd>

        ${doc}

      </dd>
    '';
in
concatStringsSep "\n" (map showBuiltin (attrNames builtinsInfo))
