const tagClassNameMap: { [key: string]: string } = {
  '[!NOTE]': 'markdown-alert-note',
  '[!TIP]': 'markdown-alert-tip',
  '[!IMPORTANT]': 'markdown-alert-important',
  '[!WARNING]': 'markdown-alert-warning',
  '[!CAUTION]': 'markdown-alert-caution',
};

function getTagClassname(tag: string): string {
  return tagClassNameMap[tag] ?? '';
}

export async function replaceBadges(document: HTMLElement): Promise<void> {
  const blockquotes = Array.from(document.querySelectorAll('blockquote'));

  for (const blockquote of blockquotes) {
    const { innerHTML } = blockquote;

    const tag = Object.keys(tagClassNameMap).find((t) => innerHTML.includes(t));

    if (tag) {
      blockquote.classList.add('markdown-alert', getTagClassname(tag));
    }
  }
}
