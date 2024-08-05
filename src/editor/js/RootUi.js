import { Dom } from "./Dom.js";
import { Editor } from "./Editor.js";

export class RootUi {
  static getDependencies() {
    return [HTMLElement, Dom, Window];
  }
  constructor(element, dom, window) {
    this.element = element;
    this.dom = dom;
    this.window = window;
  }
  
  setup(scenes) {
    this.scenes = scenes;
    this.element.innerHTML = "";
    
    const sidebar = this.dom.spawn(this.element, "UL", ["sidebar"]);
    for (const scene of scenes) {
      this.dom.spawn(sidebar, "LI", { "on-click": () => this.loadScene(scene) }, scene);
    }
    this.dom.spawn(sidebar, "LI", { "on-click": () => this.newScene() }, "NEW");
    
    const workspace = this.dom.spawn(this.element, "DIV", ["workspace"]);
  }
  
  loadScene(name) {
    this.window.fetch("/data/16-scene/" + name).then(rsp => {
      if (!rsp.ok) throw rsp;
      return rsp.text();
    }).then(serial => {
      const workspace = this.element.querySelector(".workspace");
      workspace.innerHTML = "";
      const editor = this.dom.spawnController(workspace, Editor);
      editor.setup(name, serial);
    }).catch(error => {
      this.window.console.error(error);
    });
  }
  
  newScene() {
    let id = 1;
    for (;; id++) {
      if (this.scenes.find(s => parseInt(s) === id)) continue;
      break;
    }
    const name = id.toString();
    const body = 
      "dirt 1 1 1 1 1 1 1 1 1 1\n" +
      "rabbit 5 8\n" +
      "carrot 3 8\n" + 
      "song 1\n";
    this.window.fetch("/data/16-scene/" + name, {
      method: "PUT",
      body,
    }).then(() => {
      const sidebar = this.element.querySelector(".sidebar");
      this.dom.spawn(sidebar, "LI", { "on-click": () => this.loadScene(name) }, name);
      return this.loadScene(name);
    }).catch(e => {
      this.window.console.error(e);
    });
  }
}
