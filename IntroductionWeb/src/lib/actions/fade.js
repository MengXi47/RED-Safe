import { browser } from '$app/environment';

/**
 * @param {HTMLElement} node
 * @param {IntersectionObserverInit} [options]
 */
export function fadeObserver(node, options = {}) {
	if (!browser) {
		return {};
	}

	node.classList.add('fade-in');

	const observer = new IntersectionObserver(
		(entries) => {
			entries.forEach((entry) => {
				if (entry.isIntersecting) {
					entry.target.classList.add('visible');
				} else {
					entry.target.classList.remove('visible');
				}
			});
		},
		{ threshold: 0.3, ...options }
	);

	observer.observe(node);

	return {
		destroy() {
			observer.disconnect();
		}
	};
}
