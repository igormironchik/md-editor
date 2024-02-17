import { replaceBadges } from './hightlight';
import { html } from '../test-utils/test-input';

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

  xtest('should proceed only top level nodes', () => {});

  test(`tag should be modified only if it has marker in first <p> tag on first line`, () => {
    const selector = '.modified';
    replaceBadges(document.body);
    const element = document.body.querySelector(selector);
    const p = element?.querySelector('p');

    expect(p?.querySelector('img')).toBeTruthy();
  });

  test(`tag should be ignored only if it has marker in first <p> tag on first line`, () => {
    const selector = '.not-modified';
    replaceBadges(document.body);
    const element = document.body.querySelector(selector);
    const p = element?.querySelector('p');

    expect(p?.querySelector('img')).not.toBeTruthy();
  });
});
