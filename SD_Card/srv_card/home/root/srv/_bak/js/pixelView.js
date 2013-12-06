/*

Copyright (c) 2013 -  Second Story [http://www.secondstory.com]

This code is protected under MIT license. 
For more information visit  : https://github.com/secondstory/LYT

*/

var PixelView = function(w, h, numPanels) {
	this.width = w;
	this.height = h;
	this.canvas = document.createElement('canvas');
	this.canvas.id = 'canvas';
	$('#canvas-holder').append(this.canvas);
	this.context = this.canvas.getContext('2d');
	this.context.canvas.width = w;
	this.context.canvas.height = h;
	this.pixels = [];
	this.colors = ['#f3ce00','#3dc4d8','#f85068'];

	for (var y=1; y<=h; y++) {
		for (var x=1; x<=w; x++) {
			var panelIndex = Math.floor(((x-1)/w)*numPanels);
			var localX = x - ((w/numPanels)*panelIndex) -1;
      var localY = h - y;
			var p = new Pixel(x, y, this.context, this.colors[panelIndex], panelIndex, localX,localY);
			this.pixels.push(p);
		}
	}
	var self = this;
	setInterval(function(){ self.update(); }, 33);
}

PixelView.prototype.touch = function(e) {
	var x = Math.floor(e.pageX/$(document).width() * this.width);
	var y = Math.floor(e.pageY/$(document).height() * this.height);
	var p = this.pixels[y*this.width + x];
	p.update();

		// doing pix on either side to "thicken" stroke
	if (y>0) this.pixels[(y-1)*this.width + x].update();
	if (y<this.height-1) this.pixels[(y+1)*this.width + x].update();
	return p.id;
}

PixelView.prototype.update = function() {
	var lastImage = this.context.getImageData(0,0,this.width,this.height);
	var pixelData = lastImage.data;
	for (var i=3; i<pixelData.length; i+=4) pixelData[i] -= 2;
	this.context.putImageData(lastImage, 0, 0);
}