import { replaceBadges } from './hightlight';
import { html } from './test-input';

describe('highlight util', () => {
  beforeEach(() => {
    document.body.innerHTML = html.slice();
  });

  test('should be defined', () => {
    expect(replaceBadges).toBeDefined();
    expect(typeof replaceBadges).toBe('function');
  });

  test('should not trow an error', () => {
    expect(() => replaceBadges(document.body)).not.toThrow();
  });

  test('should replace [!IMPORTANT] node', () => {
    const selector = '.markdown-alert-important';
    replaceBadges(document.body);
    const element = document.body.querySelector(selector);
    const p = element?.querySelector('p');
    const img = p?.querySelector('img');

    expect(img?.src).toBe('qrc:/res/important.svg');
  });
});
