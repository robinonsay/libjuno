/**
 * jest-esm-to-cjs.cjs
 *
 * Minimal Jest transformer for self-contained ESM bundles (no `import`
 * statements, single trailing `export { ... }` block). Used to make
 * chevrotain v11+ (ESM-only) loadable in Jest's CommonJS environment.
 *
 * Handles two export-entry forms:
 *   name          →  exports.name = name;
 *   local as name →  exports.name = local;
 */

"use strict";

module.exports = {
    process(sourceText) {
        const cjs = sourceText.replace(
            // Match the `export { ... };` block — may be multiline
            /^export \{([\s\S]*?)\};/m,
            (_, exportList) => {
                const assignments = exportList
                    .split(",")
                    .map((s) => s.trim())
                    .filter(Boolean)
                    .map((entry) => {
                        const asParts = entry.split(/\s+as\s+/);
                        if (asParts.length === 2) {
                            return `exports.${asParts[1].trim()} = ${asParts[0].trim()};`;
                        }
                        return `exports.${entry} = ${entry};`;
                    })
                    .join("\n");
                return assignments;
            },
        );
        return { code: cjs };
    },
};
