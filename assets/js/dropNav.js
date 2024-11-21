
var dropped = false;

function modifyClass(className, modiType, className2) {
  const arr = document.getElementsByClassName(className);

  switch (modiType) {
    case "add":

      for (i = 0; i < arr.length; i++) {
        arr[i].classList.add(className2);
      }
        
      break;
    case "remove":

      for (i = 0; i < arr.length; i++) {
        arr[i].classList.remove(className2);
      }

      break;
    default:
      console.log("Function was called with unknown modification type."); 
  }
}

function dropNav() {
  // Update class lists to display navigation bar drop down menu on narrow screens

  if (!dropped) {
    modifyClass("navtab", "add", "responsive");
  }
  else {
    modifyClass("navtab", "remove", "responsive");
  }

  dropped = !dropped;
}

function collapseNav() {
  // TODO: Can we avoid using a global variable? 
  dropped = true;
  dropNav();
}

