const TILESIZE = 40; // Whatever we like.
const COLC = 10; // Must match game.
const ROWC = 10; // Must match game.

export class SceneVisual {
  static getDependencies() {
    return [HTMLCanvasElement, Window];
  }
  constructor(element, window) {
    this.element = element;
    this.window = window;
    
    // Owner should replace:
    this.sceneDirty = () => {};
    
    this.scene = null;
    this.renderTimeout = null;
    this.clickables = []; // Rebuilds during render.
    this.mouseListener = null;
    this.drag = null; // clickable (not necessarily in this.clickables; they rebuild often)
    this.dragX = 0;
    this.dragY = 0;
    this.dragAnchorX = 0;
    this.dragAnchorY = 0;
    this.dragShift = false;
    
    this.element.width = TILESIZE * COLC;
    this.element.height = TILESIZE * ROWC;
    this.context = this.element.getContext("2d");
    this.element.addEventListener("mousedown", e => this.onMouseDown(e));
  }
  
  onRemoveFromDom() {
    if (this.renderTimeout) {
      this.window.clearTimeout(this.renderTimeout);
      this.renderTimeout = null;
    }
    if (this.mouseListener) {
      this.window.removeEventListener("mousemove", this.mouseListener);
      this.window.removeEventListener("mouseup", this.mouseListener);
      this.mouseListener = null;
    }
  }
  
  setup(scene) {
    this.scene = scene;
    this.renderSoon();
  }
  
  renderSoon() {
    if (this.renderTimeout) return;
    this.renderTimeout = this.window.setTimeout(() => {
      this.renderTimeout = null;
      this.renderNow();
    }, 20);
  }
  
  renderNow() {
    this.clickables = [];
    this.context.fillStyle = "#8ac";
    this.context.fillRect(0, 0, this.element.width, this.element.height);
    if (!this.scene) return;
    
    { // Dirt.
      this.context.fillStyle = "#080";
      let x = 0;
      for (const h of this.scene.dirt) {
        const y = (ROWC - h) * TILESIZE;
        if (h) this.context.fillRect(x, y, TILESIZE, h * TILESIZE);
        x += TILESIZE;
      }
    }
    
    if (this.scene.hammer) { // Optional Hammer, with variable width.
      const hammer = {
        x: this.scene.hammer[0] * TILESIZE,
        y: 0,
        w: this.scene.hammer[1] * TILESIZE,
        h: 15,
        color: "#234",
        command: "hammer x w period phase",
      };
      this.clickables.push(hammer);
      this.context.fillStyle = hammer.color;
      this.context.fillRect(hammer.x, hammer.y, hammer.w, hammer.h);
    }
    
    for (const flame of this.scene.flames) { // Multiple Flamethrowers.
      const clk = {
        x: flame[0] * TILESIZE,
        y: flame[1] * TILESIZE + 20,
        w: flame[2] * TILESIZE,
        h: 20,
        color: "#f00",
        command: "flame x y w",
        model: flame,
      };
      if (clk.w < 0) {
        clk.x += clk.w;
        clk.w = -clk.w + TILESIZE;
      }
      this.clickables.push(clk);
      this.context.fillStyle = clk.color;
      this.context.fillRect(clk.x, clk.y, clk.w, clk.h);
    }
    
    for (const platform of this.scene.platforms) { // Multiple Platforms.
      const clk = {
        x: platform[0] * TILESIZE,
        y: platform[1] * TILESIZE,
        w: platform[2] * TILESIZE,
        h: 20,
        color: "#840",
        command: "platform x y w",
        model: platform,
      };
      this.clickables.push(clk);
      this.context.fillStyle = clk.color;
      this.context.fillRect(clk.x, clk.y, clk.w, clk.h);
    }
    
    { // Rabbit. Always present, and add to clickables.
      const rabbit = {
        x: this.scene.rabbit[0] * TILESIZE + 4,
        y: this.scene.rabbit[1] * TILESIZE + 4,
        w: 20,
        h: 20,
        color: "#fff",
        command: "rabbit x y",
      };
      this.clickables.push(rabbit);
      this.context.beginPath();
      const radius = rabbit.w * 0.5;
      this.context.ellipse(rabbit.x + radius, rabbit.y + radius, radius, radius, 0, Math.PI * 2, false);
      this.context.fillStyle = rabbit.color;
      this.context.fill();
      this.context.strokeStyle = "#000";
      this.context.stroke();
    }
    
    { // Carrot. Exactly the same thing as a rabbit. (botanically speaking)
      const carrot = {
        x: this.scene.carrot[0] * TILESIZE + 20,
        y: this.scene.carrot[1] * TILESIZE + 4,
        w: 20,
        h: 20,
        color: "#f80",
        command: "carrot x y",
      };
      this.clickables.push(carrot);
      this.context.beginPath();
      const radius = carrot.w * 0.5;
      this.context.ellipse(carrot.x + radius, carrot.y + radius, radius, radius, 0, Math.PI * 2, false);
      this.context.fillStyle = carrot.color;
      this.context.fill();
      this.context.strokeStyle = "#000";
      this.context.stroke();
    }
    
    if (this.scene.crocodile) { // Crocodile. Same as rabbit and carrot, but optional.
      const crocodile = {
        x: this.scene.crocodile[0] * TILESIZE + 4,
        y: this.scene.crocodile[1] * TILESIZE + 20,
        w: 20,
        h: 20,
        color: "#0f0",
        command: "crocodile x y",
      };
      this.clickables.push(crocodile);
      this.context.beginPath();
      const radius = crocodile.w * 0.5;
      this.context.ellipse(crocodile.x + radius, crocodile.y + radius, radius, radius, 0, Math.PI * 2, false);
      this.context.fillStyle = crocodile.color;
      this.context.fill();
      this.context.strokeStyle = "#000";
      this.context.stroke();
    }
    
    // Hawk and Song do not show on the visual.
    
    { // Grid lines.
      this.context.beginPath();
      for (let i=1, x=TILESIZE+0.5, y=TILESIZE+0.5; i<10; i++, x+=TILESIZE, y+=TILESIZE) {
        this.context.moveTo(x, 0);
        this.context.lineTo(x, TILESIZE * ROWC);
        this.context.moveTo(0, y);
        this.context.lineTo(TILESIZE * COLC, y);
      }
      this.context.strokeStyle = "#000";
      this.context.stroke();
    }
  }
  
  onDrag(x, y, dx, dy) {
    const rx = x - this.dragAnchorX;
    const ry = y - this.dragAnchorY;
    switch (this.drag.command) {
      case "rabbit x y": {
          this.scene.rabbit = [this.scene.rabbit[0] + dx, this.scene.rabbit[1] + dy];
          this.sceneDirty();
          this.renderSoon();
        } break;
      case "carrot x y": {
          this.scene.carrot = [this.scene.carrot[0] + dx, this.scene.carrot[1] + dy];
          this.sceneDirty();
          this.renderSoon();
        } break;
      case "crocodile x y": {
          this.scene.crocodile = [this.scene.crocodile[0] + dx, this.scene.crocodile[1] + dy];
          this.sceneDirty();
          this.renderSoon();
        } break;
      case "platform x y w": {
          if (this.dragShift) {
            this.drag.model[2] += dx;
          } else {
            this.drag.model[0] += dx;
            this.drag.model[1] += dy;
          }
          this.sceneDirty();
          this.renderSoon();
        } break;
      case "flame x y w": {
          if (this.dragShift) {
            this.drag.model[2] += dx;
          } else {
            this.drag.model[0] += dx;
            this.drag.model[1] += dy;
          }
          this.sceneDirty();
          this.renderSoon();
        } break;
      case "hammer x w period phase": {
          if (this.dragShift) {
            this.scene.hammer[1] += dx;
          } else {
            this.scene.hammer[0] += dx;
          }
          this.sceneDirty();
          this.renderSoon();
        } break;
      default: console.log(`TODO command = ${JSON.stringify(this.drag.command)}`);
    }
  }
  
  onMoveOrUp(event) {
    if (event.type === "mouseup") {
      this.window.removeEventListener("mousemove", this.mouseListener);
      this.window.removeEventListener("mouseup", this.mouseListener);
      this.mouseListener = null;
      return;
    }
    const bounds = this.element.getBoundingClientRect();
    const x = Math.floor((event.clientX - bounds.left) / TILESIZE);
    const y = Math.floor((event.clientY - bounds.top) / TILESIZE);
    if ((x === this.dragX) && (y === this.dragY)) return;
    const dx = x - this.dragX;
    const dy = y - this.dragY;
    this.dragX = x;
    this.dragY = y;
    this.onDrag(x, y, dx, dy);
  }
  
  clickOnClickable(clickable, event) {
    this.mouseListener = e => this.onMoveOrUp(e);
    this.window.addEventListener("mousemove", this.mouseListener);
    this.window.addEventListener("mouseup", this.mouseListener);
    this.drag = clickable;
    this.dragShift = event.shiftKey;
    this.dragX = Math.floor(event.offsetX / TILESIZE);
    this.dragY = Math.floor(event.offsetY / TILESIZE);
    this.dragAnchorX = this.dragX;
    this.dragAnchorY = this.dragY;
  }
  
  onMouseDown(event) {
    if (this.mouseListener) return;
    
    // We've arranged for the framebuffer and layout size to be the same, so no scaling here:
    const x = event.offsetX;
    const y = event.offsetY;
    
    // Check clickables first, in reverse render order.
    // Most of them render as circles, but for clicking purposes they are rectangles.
    for (let i=this.clickables.length; i-->0; ) {
      const clk = this.clickables[i];
      if (x < clk.x) continue;
      if (y < clk.y) continue;
      if (x >= clk.x + clk.w) continue;
      if (y >= clk.y + clk.h) continue;
      return this.clickOnClickable(clk, event);
    }
    
    // Locate grid cell. It can't happen, but allow that it could be OOB, and do nothing.
    const col = Math.floor(x / TILESIZE);
    const row = Math.floor(y / TILESIZE);
    if ((col < 0) || (col >= COLC) || (row < 0) || (row >= TILESIZE)) return;
    
    // If they clicked in the top cell of a dirt column, zero it.
    // Otherwise move the dirt column's top to that cell.
    const top = ROWC - (this.scene.dirt[col] || 0);
    if (row === top) {
      this.scene.dirt[col] = 0;
      this.renderSoon();
      this.sceneDirty();
    } else {
      this.scene.dirt[col] = ROWC - row;
      this.renderSoon();
      this.sceneDirty();
    }
  }
}
