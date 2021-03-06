const BLOCK_TYPE = 16;
const CUT_LEVEL = 0.41;

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
    var predictions; 
    console.log(`Predicting ${imgElement.src}...`);
    const startTime = performance.now();
    var canvas = document.createElement('canvas');
    var context = canvas.getContext('2d');
    imgElement.width = imgElement.height = 320;
    //context.width = imgElement.width;
    //context.geight = imgElement.height;
    context.drawImage(imgElement, 0, 0);
    var imgData = context.getImageData(0,0,imgElement.width, imgElement.height).data;

    chrome.runtime.sendNativeMessage('com.ttroll.nomorecatz',
    { type: BLOCK_TYPE, cut: CUT_LEVEL, width: imgElement.width, height: imgElement.height, data : imgData },
	function(message) {
                //console.log(metaTags);
                const totalTime = Math.floor(performance.now() - startTime);
                console.log(`Prediction done in ${totalTime}ms:`, message.result);
    		//console.log(message);
		metaTags.result = message.result;
                if (message.result == 1) {
		  chrome.tabs.sendMessage(metaTags.tabId, {
      		    action: 'IMAGE_PROCESSED',
      		    payload: metaTags,
      		  });
                }
  	});
    predictions = chrome.extension.getURL("/nocats.png");
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