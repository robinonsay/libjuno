import { CstNode, IToken } from "chevrotain";

/** Returns the first child CstNode under the given key, or undefined. */
export function getChild(node: CstNode, key: string): CstNode | undefined {
    const children = node.children[key] as (CstNode | IToken)[] | undefined;
    if (!children || children.length === 0) { return undefined; }
    const first = children[0];
    return (first as CstNode).children !== undefined ? (first as CstNode) : undefined;
}

/** Returns ALL child CstNodes under the given key (empty array if absent). */
export function getChildren(node: CstNode, key: string): CstNode[] {
    const children = node.children[key] as (CstNode | IToken)[] | undefined;
    if (!children) { return []; }
    return children.filter((c): c is CstNode => (c as CstNode).children !== undefined);
}

/** Returns the first token under the given key, or undefined. */
export function getToken(node: CstNode, key: string): IToken | undefined {
    const children = node.children[key] as (CstNode | IToken)[] | undefined;
    if (!children || children.length === 0) { return undefined; }
    const first = children[0];
    return (first as CstNode).children === undefined ? (first as IToken) : undefined;
}

/** Returns ALL tokens under the given key (empty array if absent). */
export function getTokens(node: CstNode, key: string): IToken[] {
    const children = node.children[key] as (CstNode | IToken)[] | undefined;
    if (!children) { return []; }
    return children.filter((c): c is IToken => (c as CstNode).children === undefined);
}

/** Returns ALL child CstNodes and tokens flattened (used for mixed iteration). */
export function getAll(node: CstNode, key: string): (CstNode | IToken)[] {
    return (node.children[key] as (CstNode | IToken)[] | undefined) ?? [];
}
