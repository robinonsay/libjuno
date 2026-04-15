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
exports.JunoDefinitionProvider = void 0;
// @{"req": ["REQ-VSCODE-002", "REQ-VSCODE-005", "REQ-VSCODE-006", "REQ-VSCODE-007", "REQ-VSCODE-016"]}
const vscode = __importStar(require("vscode"));
const quickPickHelper_1 = require("./quickPickHelper");
/** Error message returned by VtableResolver when the line has no LibJuno pattern. */
const NO_PATTERN_MSG = 'No LibJuno API call pattern found at cursor position.';
/** Error message returned by FailureHandlerResolver when the line is unrelated. */
const NO_HANDLER_PATTERN_MSG = 'Line does not contain a failure handler reference.';
/**
 * VSCode DefinitionProvider that resolves LibJuno vtable call sites and
 * failure handler references to their concrete implementation locations.
 * Registered for C and C++ documents (REQ-VSCODE-007).
 */
class JunoDefinitionProvider {
    constructor(vtableResolver, failureHandlerResolver, index) {
        this.vtableResolver = vtableResolver;
        this.failureHandlerResolver = failureHandlerResolver;
        this.index = index;
    }
    async provideDefinition(document, position, token) {
        const lineText = document.lineAt(position.line).text;
        const file = document.uri.fsPath;
        const line = position.line + 1; // convert to 1-based
        const column = position.character;
        // 2. Try failure handler resolver first (design §6.2 priority)
        let result = this.failureHandlerResolver.resolve(file, line, column, lineText);
        // 3. Fall through to vtable resolver if failure handler pattern not present
        if (!result.found && result.errorMsg === NO_HANDLER_PATTERN_MSG) {
            result = this.vtableResolver.resolve(file, line, column, lineText);
        }
        if (!result.found || result.locations.length === 0) {
            // Only surface an error when a recognizable pattern existed but resolution failed
            const msg = result.errorMsg ?? '';
            if (msg && msg !== NO_PATTERN_MSG && msg !== NO_HANDLER_PATTERN_MSG) {
                vscode.window.setStatusBarMessage(`$(warning) LibJuno: Could not resolve implementation — ${msg}`, 5000);
            }
            return undefined;
        }
        // 4a. Single location: return directly for native VSCode navigation (REQ-VSCODE-005)
        if (result.locations.length === 1) {
            return toVscodeLocations(result.locations);
        }
        // 4b. Multiple locations: show QuickPick and navigate imperatively (REQ-VSCODE-006)
        token.onCancellationRequested(() => { });
        const selected = await (0, quickPickHelper_1.showImplementationQuickPick)(result.locations);
        if (selected) {
            await navigateTo(selected);
        }
        return undefined;
    }
}
exports.JunoDefinitionProvider = JunoDefinitionProvider;
function toVscodeLocations(locations) {
    return locations.map(loc => new vscode.Location(vscode.Uri.file(loc.file), new vscode.Position(Math.max(0, loc.line - 1), 0)));
}
async function navigateTo(loc) {
    try {
        const uri = vscode.Uri.file(loc.file);
        const pos = new vscode.Position(Math.max(0, loc.line - 1), 0);
        await vscode.window.showTextDocument(uri, {
            selection: new vscode.Range(pos, pos),
        });
    }
    catch (err) {
        console.log('[LibJuno] Failed to navigate to definition:', err);
    }
}
//# sourceMappingURL=junoDefinitionProvider.js.map