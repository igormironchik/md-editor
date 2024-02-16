const tagClassNameMap: { [key: string]: string } = {
  '[!NOTE]': 'markdown-alert-note',
  '[!TIP]': 'markdown-alert-tip',
  '[!IMPORTANT]': 'markdown-alert-important',
  '[!WARNING]': 'markdown-alert-warning',
  '[!CAUTION]': 'markdown-alert-caution',
};

const tagImageMap: { [key: string]: string } = {
  '[!NOTE]': 'note',
  '[!TIP]': 'tip',
  '[!IMPORTANT]': 'important',
  '[!WARNING]': 'warning',
  '[!CAUTION]': 'caution',
};

function getTagClassname(tag: string): string {
  return tagClassNameMap[tag] ?? '';
}

function createImageParagraph(tag: string): DocumentFragment {
  const documentFragment = document.createDocumentFragment();
  const p = document.createElement('p');
  const img = document.createElement('img');
  img.src = `qrc:/res/${tagImageMap[tag] ?? ''}.svg`;
  p.append(img);
  documentFragment.append(p);

  return documentFragment;
}

export function replaceBadges(document: HTMLElement) {
  const blockquotes = Array.from(document.querySelectorAll('blockquote'));

  for (const blockquote of blockquotes) {
    const { innerHTML } = blockquote;

    const tag = Object.keys(tagClassNameMap).find((t) => innerHTML.includes(t));

    if (tag) {
      blockquote.innerHTML = blockquote.innerHTML.replace(tag, '');
      blockquote.prepend(createImageParagraph(tag));
      blockquote.classList.add('markdown-alert', getTagClassname(tag));
    }
  }
}
