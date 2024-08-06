export class Scene {
  constructor(serial) {
  
    this.dirt = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    this.rabbit = [0, 0]; // x,y
    this.carrot = [1, 0]; // x,y
    this.crocodile = null; // [x,y] or null
    this.hawk = false;
    this.song = 1;
    this.hammer = null; // [x,w,period,phase] or null
    this.flames = []; // [x,y,w]
    this.platforms = []; // [x,y,w]
    this.invalid = []; // strings, full lines from serial that we couldn't process
    
    for (const line of serial.split("\n")) {
      const words = line.split("#")[0].split(/\s+/g).filter(v => v);
      if (!words.length) continue;
      const argv = words.slice(1).map(v => +v);
      switch (words[0]) {
        case "dirt": this.dirt = argv; break;
        case "rabbit": this.rabbit = argv; break;
        case "carrot": this.carrot = argv; break;
        case "crocodile": this.crocodile = argv; break;
        case "hawk": this.hawk = true; break;
        case "song": this.song = argv[0]; break;
        case "flame": this.flames.push(argv); break;
        case "platform": this.platforms.push(argv); break;
        case "hammer": this.hammer = argv; break;
        default: this.invalid.push(line);
      }
    }
  }
  
  encode() {
    let dst = "";
    dst += "dirt " + this.dirt.join(" ") + "\n";
    dst += "rabbit " + this.rabbit.join(" ") + "\n";
    dst += "carrot " + this.carrot.join(" ") + "\n";
    dst += `song ${this.song || 0}\n`;
    if (this.crocodile) dst += "crocodile " + this.crocodile.join(" ") + "\n";
    if (this.hawk) dst += "hawk\n";
    if (this.hammer) dst += "hammer " + this.hammer.join(" ") + "\n";
    for (const flame of this.flames) {
      dst += "flame " + flame.join(" ") + "\n";
    }
    for (const platform of this.platforms) {
      dst += "platform " + platform.join(" ") + "\n";
    }
    return dst;
  }
}
