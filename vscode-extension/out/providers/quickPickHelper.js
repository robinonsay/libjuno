"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
exports.showImplementationQuickPick = showImplementationQuickPick;
// @{"req": ["REQ-VSCODE-006"]}
const vscode = __importStar(require("vscode"));
const path = __importStar(require("path"));
/**
 * Displays a QuickPick list for selecting among multiple concrete
 * implementation locations (Section 6.3 / REQ-VSCODE-006).
 *
 * @param results Non-empty list of concrete implementation locations.
 * @returns The selected location, or `undefined` if the picker was dismissed.
 */
async function showImplementationQuickPick(results) {
    const items = results.map(loc => ({
        label: loc.functionName,
        description: `${path.basename(loc.file)}:${loc.line}`,
        detail: loc.file,
        location: loc,
    }));
    const chosen = await vscode.window.showQuickPick(items, {
        placeHolder: 'Select an implementation to navigate to',
        matchOnDescription: true,
        matchOnDetail: true,
    });
    return chosen?.location;
}
//# sourceMappingURL=quickPickHelper.js.map