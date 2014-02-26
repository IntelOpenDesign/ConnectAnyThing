/*

Copyright (c) 2013 -  Second Story [http://www.secondstory.com]

This code is protected under MIT license. 
For more information visit  : https://github.com/secondstory/LYT

*/
 
$(document).ready(function(){
  if (!"WebSocket" in window) {
    alert("LYT requires a modern browser.");
    return;
  }

  var bMobile = ( /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent) );


  var socketController = new SocketController('192.168.0.10', 80);
  var pixelView = new PixelView(18, 55, 3, socketController.socket);

  var bounce = function() { window.location.assign('http://secondstory.com'); }

  if (bMobile) {
    document.body.addEventListener('touchmove', function(e){
      for (var i=0; i<e.touches.length; i++)
        socketController.send(pixelView.touch(e.touches[i]));
      e.preventDefault();
    });
    $('#logo2s').on('touchstart', bounce);
  }

  else {
    document.body.addEventListener('mousemove', function(e){
      socketController.send(pixelView.touch(e));
    });
    $('#logo2s').on('mousedown', bounce);
  }

  
  $('.intro').fadeIn(300).delay(1000).fadeOut( "slow", function() {
    // Animation complete.
    window.scrollTo(0, 0);
    $('.intro').remove();
  });
});