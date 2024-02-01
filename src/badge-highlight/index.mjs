import {readFile, open} from "node:fs/promises";
import {JSDOM} from 'jsdom';
import path from "path";
import {fileURLToPath} from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const FILES_DIR = `${__dirname}/files/`;
const FILE_PATH = `${__dirname}/files/input.html`;
const ERROR_MESSAGE = "FS operation failed";

const tagClassNameMap = {
    '[!NOTE]': 'markdown-alert-note',
    '[!TIP]': 'markdown-alert-tip',
    '[!IMPORTANT]': 'markdown-alert-important',
    '[!WARNING]': 'markdown-alert-warning',
    '[!CAUTION]': 'markdown-alert-caution',
}

function getTagClassname(tag) {
    return tagClassNameMap[tag] ?? '';
}

async function replaceBadges() {
    const content = await readFile(`${FILE_PATH}`, {
        encoding: "utf8",
    });
    const dom = new JSDOM(content);
    const {document} = dom.window;
    const blockquotes = [...document.querySelectorAll('blockquote')];

    for (const blockquote of blockquotes) {
        const {innerHTML} = blockquote;

        const tag = Object.keys(tagClassNameMap).find(t => innerHTML.includes(t));

        if (tag) {
            blockquote.classList.add('markdown-alert', getTagClassname(tag));
        }
    }

    let outputFile;
    try {
        outputFile = await open(`${FILES_DIR}/output.html`, "w+");
        outputFile.write(dom.serialize());
    } catch (e) {
        if (e.code === "EEXIST") {
            throw ERROR_MESSAGE;
        }
    } finally {
        outputFile?.close();
    }
}

await replaceBadges();
