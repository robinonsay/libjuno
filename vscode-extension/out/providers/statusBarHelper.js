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
exports.StatusBarHelper = void 0;
// @{"req": ["REQ-VSCODE-013", "REQ-VSCODE-001"]}
const vscode = __importStar(require("vscode"));
/**
 * Manages the LibJuno status bar item for indexed file counts and transient
 * resolution-failure messages (Section 8.1).
 */
class StatusBarHelper {
    constructor() {
        this.item = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left);
        this.item.show();
    }
    /**
     * Shows "LibJuno: Indexed N files" in the status bar.
     * @param fileCount Number of indexed source files.
     */
    showIndexed(fileCount) {
        this.cancelClear();
        this.item.text = `$(check) LibJuno: Indexed ${fileCount} files`;
        this.item.tooltip = 'LibJuno workspace index is ready';
        this.item.show();
    }
    /**
     * Shows an error icon and message. The message auto-clears after 5 seconds,
     * restoring the last indexed-count display (Section 8.1).
     * @param message Human-readable error description.
     */
    showError(message) {
        this.cancelClear();
        const previous = this.item.text;
        this.item.text = `$(warning) LibJuno: ${message}`;
        this.item.show();
        this.clearTimer = setTimeout(() => {
            this.item.text = previous;
        }, 5000);
    }
    dispose() {
        this.cancelClear();
        this.item.dispose();
    }
    cancelClear() {
        if (this.clearTimer !== undefined) {
            clearTimeout(this.clearTimer);
            this.clearTimer = undefined;
        }
    }
}
exports.StatusBarHelper = StatusBarHelper;
//# sourceMappingURL=statusBarHelper.js.map