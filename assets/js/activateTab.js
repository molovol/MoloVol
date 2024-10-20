
function activateTab() {

  const path = window.location.pathname;
  let page = path.split("/").pop();
  if (page === "") {
    page = "home";
  }

  const x = document.getElementsByClassName("navtab");
  let i;
  for (i = 0; i < x.length; i++) {
    x[i].classList.remove("active");
  }
  document.getElementById("nav-" + page).classList.add("active");
}

activateTab();
