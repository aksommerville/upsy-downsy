export class Scene {
  constructor(serial) {
    if (typeof(serial) !== "string") throw new Error(`expected string`);
    this.serial = serial;
    this.fields = this.decodeFields(this.serial); // Each is an array of [String, Number...]
    this.objects = this.decodeObjects(this.fields); // {x,y,w,h,key,mut:"xywh"}
  }
  
  decodeFields(src) {
    const fields = [];
    for (let srcp=0, lineno=1; srcp<src.length; lineno++) {
      let nlp = src.indexOf("\n", srcp);
      if (nlp < 0) nlp = src.length;
      const line = src.substring(srcp, nlp).trim();
      srcp = nlp + 1;
      if (!line || line.startsWith("#")) continue;
      const words = line.split(/\s+/g).filter(v => v);
      if (!words.length) continue;
      for (let i=1; i<words.length; i++) {
        const v = +words[i];
        if (isNaN(v)) throw new Error(`${lineno}: Expected integer, found ${JSON.stringify(words[i])}`);
        words[i] = v;
      }
      fields.push(words);
    }
    return fields;
  }
  
  decodeObjects(fields) {
    const objects = [];
    for (const field of fields) {
      switch (field[0]) {

        case "dirt": { // One object per nonzero column.
            for (let col=0; col<10; col++) {
              const h = field[1 + col];
              if (!h) continue;
              objects.push({ key: "dirt", x: col, y: 10 - h, w: 1, h, mut: "h", field });
            }
          } break;
          
        case "rabbit": // Simple [x,y]
        case "carrot":
        case "crocodile": {
            objects.push({ key: field[0], x: field[1], y: field[2], w: 1, h: 1, mut: "xy", field });
          } break;
          
        case "platform": // [x,y,w]
        case "flame": {
            objects.push({ key: field[0], x: field[1], y: field[2], w: field[3], h: 1, mut: "xyw", field });
          } break;
          
        case "hammer": { // [x,w]
            objects.push({ key: "hammer", x: field[1], y: 0, w: field[2], h: 1, mut: "xw", field });
          } break;
          
        // Things that don't have any stored geometry, just pick a place.
        case "song": objects.push({ key: "song", x: 9, y: 1, w: 1, h: 1, mut: "", field }); break;
        case "hawk": objects.push({ key: "hawk", x: 9, y: 0, w: 1, h: 1, mut: "", field }); break;
          
        default: console.warn(`Unknown field ${JSON.stringify(field)}`);
      }
    }
    return objects;
  }
  
  encode() {
    return this.fields.map(v => v.join(" ")).join("\n");
  }
  
  getField(key) {
    return this.fields.find(fld => fld[0] === key);
  }
  
  objectAtPoint(x, y) {
    for (const object of this.objects) {
      if (object.w > 0) {
        if (x < object.x) continue;
        if (x >= object.x + object.w) continue;
      } else if (object.w < 0) {
        if (x < object.x + object.w) continue;
        if (x > object.x) continue;
      } else continue;
      if (object.h > 0) {
        if (y < object.y) continue;
        if (y >= object.y + object.h) continue;
      } else if (object.h < 0) {
        if (y < object.y + object.h) continue;
        if (y > object.y) continue;
      } else continue;
      return object;
    }
    return null;
  }
  
  /* Find the corresponding entry in (this.fields) and rewrite it.
   * We assume that (object) is from our own (this.objects).
   */
  replaceFieldForObject(object) {
    if (!object || !object.field) return;
    if (object.key === "dirt") {
      object.field[1 + object.x] = object.h;
    } else switch (object.mut) {
      case "xywh": object.field[1] = object.x; object.field[2] = object.y; object.field[3] = object.w; object.field[4] = object.h; break;
      case "xyw": object.field[1] = object.x; object.field[2] = object.y; object.field[3] = object.w; break;
      case "xyh": object.field[1] = object.x; object.field[2] = object.y; object.field[3] = object.h; break;
      case "xw": object.field[1] = object.x; object.field[2] = object.w; break;
      case "xy": object.field[1] = object.x; object.field[2] = object.y; break;
    }
  }
  
  addField(key, x, y) {
    let field = null;
    switch (key) {
       case "dirt": break;
       case "rabbit": this.removeAll("rabbit"); this.fields.push(field = ["rabbit", x, y]); this.objects.push({ key, x, y, w: 1, h: 1, mut: "xy", field }); break;
       case "carrot": this.removeAll("carrot"); this.fields.push(field = ["carrot", x, y]); this.objects.push({ key, x, y, w: 1, h: 1, mut: "xy", field }); break;
       case "crocodile": this.removeAll("crocodile"); this.fields.push(field = ["crocodile", x, y]); this.objects.push({ key, x, y, w: 1, h: 1, mut: "xy", field }); break;
       case "platform": this.fields.push(field = ["platform", x, y, 1]); this.objects.push({ key, x, y, w: 1, h: 1, mut: "xyw", field }); break;
       case "flame": this.fields.push(field = ["flame", x, y, 1]); this.objects.push({ key, x, y, w: 1, h: 1, mut: "xyw", field }); break;
       case "hammer": this.removeAll("hammer"); this.fields.push(field = ["hammer", x, w]); this.objects.push({ key, x, y: 0, w: 1, h: 1, mut: "xw", field }); break;
       case "song": this.removeAll("song"); this.fields.push(field = ["song", 0]); this.objects.push({ key, x: 9, y: 1, w: 1, h: 1, mut: "", field }); break;
       case "hawk": this.removeAll("hawk"); this.fields.push(field = ["hawk"]); this.objects.push({ key, x: 9, y: 0, w: 1, h: 1, mut: "", field }); break;
    }
  }
  
  removeAll(key) {
    this.fields = this.fields.filter(f => f[0] !== key);
    this.objects = this.objects.filter(o => o.key !== key);
  }
}
