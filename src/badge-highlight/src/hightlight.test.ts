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
});
