const CACHE_NAME = 'drone-gcs-mobile-v1';
const ASSETS = [
  './',
  './DroneGCS_Mobile_App.html',
  './TestScreen_Mobile_App.html',
  './manifest.webmanifest',
  './icon-192.svg',
  './icon-512.svg'
];
self.addEventListener('install', event => {
  event.waitUntil(caches.open(CACHE_NAME).then(cache => cache.addAll(ASSETS)).catch(()=>{}));
  self.skipWaiting();
});
self.addEventListener('activate', event => {
  event.waitUntil(caches.keys().then(keys => Promise.all(keys.filter(k => k !== CACHE_NAME).map(k => caches.delete(k)))));
  self.clients.claim();
});
self.addEventListener('fetch', event => {
  const url = new URL(event.request.url);
  if (url.hostname === '192.168.4.1' || url.hostname === '127.0.0.1') return;
  event.respondWith(caches.match(event.request).then(cached => cached || fetch(event.request)));
});
