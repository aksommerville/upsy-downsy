import { Injector } from "./js/Injector.js";
import { Dom } from "./js/Dom.js";
import { RootUi } from "./js/RootUi.js";

window.addEventListener("load", () => {
  const injector = new Injector(window, document);
  const dom = injector.getInstance(Dom);
  fetch("/data/16-scene").then(rsp => {
    if (!rsp.ok || (rsp.headers.get("X-Is-Directory") !== "true")) throw rsp;
    return rsp.json();
  }).then((scenes) => {
    const rootUi = dom.spawnController(document.body, RootUi);
    rootUi.setup(scenes);
  }).catch((error) => {
    console.error(error);
  });
}, { once: true });
