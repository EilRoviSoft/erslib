# Localization

`ers::LocaleTable` (`erslib/core/localization.hpp`) reads `<root>/<locale>/<domain>/...`, `<root>/<locale>/<domain>.l10n.*` and `<root>/_common/...`, and builds an immutable `Domain` per name. Texts are found by locale, then the common texts, then the fallback locale.


## References

A domain can say that one text is another one:

    "blue":          { "hex": "#1F4FA3", "name@ru": "синий" },
    "stripped-blue": { "hex": "$blue",   "name@ru": "синяя полоска" }

    table.enable_references("colors");      // off by default, per domain

| Text | Means |
|---|---|
| `$blue` | the same field of the sibling `blue`, on the same level: for `stripped-blue.hex` it's `blue.hex`, for `g.stripped.hex` it's `g.blue.hex`. The key needs a group, otherwise it's an error |
| `$blue.name` | the key `blue.name` of the domain, as written |
| `$.brand` | the root-level key `brand` |
| `$$5` | the plain text `$5` |

- References are replaced while the domain is built, so a reader never sees a `$` and nothing is resolved at runtime.
- The target is read the way a reader would read it in the same locale: the locale's text, then common, then fallback.
- Chains work and files may name keys in any order. An unknown target, a cycle or a chain deeper than `reference_options_t::max_depth` (8) make the domain invalid: `on_invalid` decides if the reload fails or the previous version stays. Validators and adapters get the resolved texts.
- `enable_references` before the first `reload`; for a loaded domain the next reload builds it again.
- `reference_options_t::marker` changes the `$`.
