/**
 * Expect a promise to throw an error with a specific message.
 * @param promise - The promise to await.
 * @param {string} errorMsg - The error message that we expect to see.
 */
export const expectToThrow = async (promise: Promise<any>, errorMsg?: string) => {
    try {
        await promise
        expect(true).toBeFalsy();
    } catch (e: any) {
        if ( errorMsg ) expect(e.message).toMatch(errorMsg)
        else expect(false).toBeFalsy()
    }
}

export const mapToObject = (array: Array<{key: string, value: string}>) => {
    const obj: {[key: string]: string} = {};
    for ( const { key, value } of array ) {
        obj[key] = value;
    }
    return obj;
}

/**
 * Wait using Promise
 * @param {number} ms - timeout period
 * @example
 *
 * async wait(500);
 * //=> setTimeout using 500ms
 */
export const wait = (ms: number) => new Promise(resolve => setTimeout(() => resolve(0), ms));