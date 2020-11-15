const BLOCK_TYPE = 16;

class BackgroundProcessing {

  constructor() {
    this.imageRequests = {};
    this.addListeners();
  }

  addListeners() {
    chrome.webRequest.onCompleted.addListener(req => {
      if (req && req.tabId > 0) {
        this.imageRequests[req.url] = this.imageRequests[req.url] || req;
        this.analyzeImage(req.url);
      }
    }, { urls: ["<all_urls>"], types: ["image", "object"] });
  }

  async loadImage(src) {
    return new Promise(resolve => {
      var img = document.createElement('img');
      img.crossOrigin = "anonymous";
      img.onerror = function(e) {
        resolve(null);
      };
      img.onload = function(e) {
        if ((img.height && img.height > 64) && (img.width && img.width > 64)) {
          resolve(img);
        }
        resolve(null);
      }
      img.src = src;
    });
  }

  async predict(imgElement, metaTags) {
    console.log('Predicting...');
    const startTime = performance.now();
    var canvas = document.createElement('canvas');
    var context = canvas.getContext('2d');
    context.drawImage(imgElement, 0, 0);
    var imgData = context.getImageData(0,0,imgElement.width, imgElement.height).data;
    var predictions;

    chrome.runtime.sendNativeMessage('com.ttroll.nomorecatz',
    { type: BLOCK_TYPE, width: imgElement.width, height: imgElement.height, data : imgData },
	function(message) {
                //console.log(metaTags);
                const totalTime = Math.floor(performance.now() - startTime);
                console.log(`Prediction done in ${totalTime}ms:`, message.result);
    		//console.log(message);
		metaTags.result = message.result;
		chrome.tabs.sendMessage(metaTags.tabId, {
      		    action: 'IMAGE_PROCESSED',
      		    payload: metaTags,
      		});
  	});
    predictions = chrome.extension.getURL("/nocats.png");
    //const totalTime = Math.floor(performance.now() - startTime);
    //console.log(`Prediction done in ${totalTime}ms:`, predictions);
    return predictions;
  }

  async analyzeImage(src) {

    var meta = this.imageRequests[src];
    //console.log(meta);
    if (meta && meta.tabId) {
      if (!meta.predictions) {
        const img = await this.loadImage(src);
        if (img) {
          meta.predictions = await this.predict(img, meta);
        }
      }
    }
  }
}

var bg = new BackgroundProcessing();