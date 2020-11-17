var imageMeta = {};
const srcChecker = /url\(\s*?['"]?\s*?(\S+?)\s*?["']?\s*?\)/i

const setImageTitles = () => {
  const images = document.images;
  const keys = Object.keys(imageMeta);
  const nodes = document.querySelectorAll('*');
  for(u = 0; u < keys.length; u++) {
    var url = keys[u];
    var meta = imageMeta[url];
    for (i = 0; i < images.length; i++) {
      var img = images[i];
      if (img.src === meta.url) {
       if (meta.result == 1) {
        img.title = "Blocked by NoMoreCatZ";
        //img.src + `:\n\n${img.title}\n\n` + JSON.stringify(meta.predictions);
        img.src = meta.predictions;
        delete keys[u];
        delete imageMeta[url];
       }
      }
    }
    for (i = 0; i < nodes.length; i++) {
      let prop = window.getComputedStyle(nodes[i], null)
          .getPropertyValue('background-image');
      let match = srcChecker.exec(prop)
        if (match) {
          if (match[1] == meta.url) {
            if (meta.result == 1) {
               nodes[i].style.backgroundImage = "url('" + meta.predictions + "')";
               delete keys[u];
               delete imageMeta[url];
            }
          }
	}
    }
  }
  
}

chrome.runtime.onMessage.addListener((message, sender, sendResponse) => {
  if (message && message.payload && message.action === 'IMAGE_PROCESSED') {
    const { payload } = message;
    if (payload && payload.url) {
      imageMeta[payload.url] = payload;
      setImageTitles();
    }
  }
});

window.addEventListener('load', setImageTitles, false);
