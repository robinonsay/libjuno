/// <reference types="jest" />

// @{"verify": ["REQ-VSCODE-021"]}
import { C_FILE_EXTENSIONS } from '../workspaceIndexer';

describe("C/C++ File Extension Support (REQ-VSCODE-021)", () => {
    it("should contain the .c extension", () => {
        expect(C_FILE_EXTENSIONS.has(".c")).toBe(true);
    });

    it("should contain the .h extension", () => {
        expect(C_FILE_EXTENSIONS.has(".h")).toBe(true);
    });

    it("should contain the .cpp extension", () => {
        expect(C_FILE_EXTENSIONS.has(".cpp")).toBe(true);
    });

    it("should contain the .hpp extension", () => {
        expect(C_FILE_EXTENSIONS.has(".hpp")).toBe(true);
    });

    it("should contain the .hh extension", () => {
        expect(C_FILE_EXTENSIONS.has(".hh")).toBe(true);
    });

    it("should contain the .cc extension", () => {
        expect(C_FILE_EXTENSIONS.has(".cc")).toBe(true);
    });

    it("should contain all 6 required C/C++ extensions", () => {
        const required = [".c", ".h", ".cpp", ".hpp", ".hh", ".cc"];
        for (const ext of required) {
            expect(C_FILE_EXTENSIONS.has(ext)).toBe(true);
        }
        expect(C_FILE_EXTENSIONS.size).toBeGreaterThanOrEqual(6);
    });

    it("should NOT contain non-C/C++ extensions such as .ts, .py, and .java", () => {
        expect(C_FILE_EXTENSIONS.has(".ts")).toBe(false);
        expect(C_FILE_EXTENSIONS.has(".py")).toBe(false);
        expect(C_FILE_EXTENSIONS.has(".java")).toBe(false);
    });
});
