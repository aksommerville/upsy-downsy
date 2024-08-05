import { Dom } from "./Dom.js";
import { Scene } from "./Scene.js";

export class Editor {
  static getDependencies() {
    return [HTMLElement, Dom, Window];
  }
  constructor(element, dom, window) {
    this.element = element;
    this.dom = dom;
    this.window = window;
    this.name = "";
    this.serial = "";
    this.model = null;
    this.canvas = null;
    this.context = null;
    this.renderTimeout = null;
    this.resizeObserver = new this.window.ResizeObserver(e => this.onResize(e));
    this.mouseListener = null;
    this.mapBounds = null; // x,y,w,h in client coordinates (ie matching mouse events)
    this.dragObject = null; // belonging to this.model.objects
    this.dragMode = ""; // "" "x" "y" "xy" "w" "h" "wh" "dirt"
    this.dragAnchor = [0, 0]; // (x,y), in tiles, where the event started.
    this.dragX = 0;
    this.dragY = 0;
    
    this.colorForObject = {
      dirt: "#080",
      rabbit: "#fff",
      carrot: "#f80",
      song: "#00f",
      hammer: "#024",
      crocodile: "#0f0",
      hawk: "#ff0",
      platform: "#444",
      flame: "#f00",
    };
  }
  
  onRemoveFromDom() {
    if (this.resizeObserver) {
      this.resizeObserver.disconnect();
    }
    if (this.renderTimeout) {
      this.window.clearTimeout(this.renderTimeout);
      this.renderTimeout = null;
    }
    if (this.mouseListener) {
      this.window.removeEventListener("mouseup", this.mouseListener);
      this.window.removeEventListener("mousemove", this.mouseListener);
      this.mouseListener = null;
    }
  }
  
  setup(name, serial) {
    this.name = name;
    this.serial = serial;
    this.model = new Scene(this.serial);
    this.buildUi();
  }
  
  buildUi() {
    this.element.innerHTML = "";
    this.canvas = this.dom.spawn(this.element, "CANVAS", { "on-mousedown": e => this.onMouseDown(e) });
    this.context = this.canvas.getContext("2d");
    this.resizeObserver.observe(this.canvas);
    this.renderSoon();
  }
  
  onResize(e) {
    const bounds = this.canvas.getBoundingClientRect();
    this.canvas.width = bounds.width;
    this.canvas.height = bounds.height;
    this.renderSoon();
  }
  
  renderSoon() {
    if (this.renderTimeout) return;
    this.renderTimeout = this.window.setTimeout(() => {
      this.renderTimeout = null;
      this.renderNow();
    }, 50);
  }
  
  renderNow() {
    this.context.clearRect(0, 0, this.canvas.width, this.canvas.height);
    if (!this.model) return;
    const smallAxis = Math.min(this.canvas.width, this.canvas.height);
    const tilesize = Math.floor(smallAxis / 10);
    if (tilesize < 1) return;
    const dstx = (this.canvas.width >> 1) - (tilesize * 5);
    const dsty = (this.canvas.height >> 1) - (tilesize * 5);
    const bounds = this.canvas.getBoundingClientRect();
    this.mapBounds = { x: bounds.left + dstx, y: bounds.top + dsty, w: tilesize * 10, h: tilesize * 10 };
    
    { // Fill background.
      this.context.fillStyle = "#8ac";
      this.context.fillRect(dstx, dsty, tilesize * 10, tilesize * 10);
    }
    
    { // Objects.
      for (const object of this.model.objects) {
        let x = dstx + object.x * tilesize;
        let y = dsty + object.y * tilesize;
        let w = object.w * tilesize;
        let h = object.h * tilesize;
        if (w < 0) { x += tilesize; w -= tilesize; }
        if (h < 0) { y += tilesize; h -= tilesize; }
        this.context.fillStyle = this.colorForObject[object.key] || "#888";
        this.context.fillRect(x, y, w, h);
      }
    }
    
    { // Grid lines.
      this.context.beginPath();
      for (let i=1; i<10; i++) {
        const x = dstx + i * tilesize;
        this.context.moveTo(x, dsty);
        this.context.lineTo(x, dsty + tilesize * 10);
        const y = dsty + i * tilesize;
        this.context.moveTo(dstx, y);
        this.context.lineTo(dstx + tilesize * 10, y);
      }
      this.context.strokeStyle = "#000";
      this.context.stroke();
    }
  }
  
  onClickNowhere(x, y) {
    this.dom.modalInput("Key", "").then(v => {
      v = v?.trim?.();
      if (!v) throw null;
      this.model.addField(v, x, y);
      this.serial = this.model.encode();
      this.renderSoon();
      this.onDirty();
    }).catch(() => {});
  }
  
  openObjectModal(object) {
    const pre = object.field.map(v => v.toString()).join(" ");
    this.dom.modalInput("Field", pre).then(v => {
      if (!v) throw null;
      v = v.split(/\s+/g);
      object.field[0] = v[0];
      for (let i=1; i<object.field.length; i++) {
        object.field[i] = +v[i] || 0;
      }
      // Can't rebuild Scene.objects from a changed field, just rebuild the whole thing after reencoding.
      this.serial = this.model.encode();
      this.model = new Scene(this.serial);
      this.onDirty();
      this.renderSoon();
    }).catch(() => {});
  }
  
  onMouseUpOrMove(event) {
    if (event.type === "mouseup") {
      this.window.removeEventListener("mousemove", this.mouseListener);
      this.window.removeEventListener("mouseup", this.mouseListener);
      this.model.replaceFieldForObject(this.dragObject);
      this.mouseListener = null;
      this.dragObject = null;
      this.dragMode = "";
      this.serial = this.model.encode();
      this.onDirty();
      return;
    }
    const tilesize = this.mapBounds.w / 10;
    let x = Math.floor((event.x - this.mapBounds.x) / tilesize);
    let y = Math.floor((event.y - this.mapBounds.y) / tilesize);
    if (x < 0) x = 0; else if (x >= 10) x = 9;
    if (y < 0) y = 0; else if (y >= 10) y = 9;
    if ((x === this.dragX) && (y === this.dragY)) return;
    this.dragX = x;
    this.dragY = y;
    switch (this.dragMode) {
      case "xy": this.dragObject.x = x; this.dragObject.y = y; break;
      case "x": this.dragObject.x = x; break;
      case "y": this.dragObject.y = y; break;
      case "wh": this.dragObject.w = x - this.dragObject.x; this.dragObject.h = y - this.dragObject.y; break;
      case "w": this.dragObject.w = x - this.dragObject.x; break;
      case "h": this.dragObject.h = y - this.dragObject.y; break;
      case "dirt": this.dragObject.y = y + 1; this.dragObject.h = 9 - y; break;
    }
    this.renderSoon();
  }
  
  onMouseDown(event) {
    if (this.mouseListener) return;
    if (!this.mapBounds) return;
    if (!this.model) return;
    const tilesize = this.mapBounds.w / 10;
    const x = Math.floor((event.x - this.mapBounds.x) / tilesize);
    const y = Math.floor((event.y - this.mapBounds.y) / tilesize);
    if ((x < 0) || (y < 0) || (x >= 10) || (y >= 10)) return;
    const object = this.model.objectAtPoint(x, y);
    if (!object) {
      return this.onClickNowhere(x, y);
    }
    
    // Control to open a modal, for any object.
    if (event.ctrlKey) {
      return this.openObjectModal(object);
    }
    
    // "dirt" is special; you can only modify height but you modify it upward.
    if (object.key === "dirt") {
      this.dragMode = "dirt";
    
    // Shift to change size, anchored at top left.
    } else if (event.shiftKey) {
      if (object.mut.includes("w")) {
        if (object.mut.includes("h")) this.dragMode = "wh";
        else this.dragMode = "w";
      } else if (object.mut.includes("h")) this.dragMode = "h";
      else return;
      
    // Any other drag to change position.
    } else {
      if (object.mut.includes("x")) {
        if (object.mut.includes("y")) this.dragMode = "xy";
        else this.dragMode = "x";
      } else if (object.mut.includes("y")) this.dragMode = "y";
      else return;
      
    }
    this.dragObject = object;
    this.dragAnchor = [x, y];
    this.dragX = x;
    this.dragY = y;
    this.mouseListener = e => this.onMouseUpOrMove(e);
    this.window.addEventListener("mousemove", this.mouseListener);
    this.window.addEventListener("mouseup", this.mouseListener);
  }
  
  onDirty() {
    this.window.fetch("/data/16-scene/" + this.name, {
      method: "PUT",
      body: this.serial,
    }).then(() => {
    }).catch((error) => {
      this.window.console.error(error);
    });
  }
}
