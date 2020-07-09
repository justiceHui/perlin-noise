const GRAY = 0;
const COLOR = 1;
const HIGHLIGHT = 2;

function get_color(v, flag){
    v += 1; v /= 2.0; v *= 255;
    let r, g, b;
    if(flag === GRAY) r = g = b = v;
    if(flag === COLOR){
        r = 0; g = v; b = 255-v;
    }
    if(flag === HIGHLIGHT){
        r = 0; g = v; b = 255-v;
        if(Math.abs(0.5 - v/255) > 0.1){
            v /= 255;
            let t = 4*(v-1)*(v-1)*(v-1)+v+1; t /= 2;
            t *= 255;
            r = 0; g = t; b = 255-t;
        }
    }
    //if(flag === HIGHLIGHT && v > 0.6*255) return `rgb(0,50,0)`;
    //if(flag === HIGHLIGHT && v < 0.4*255) return `rgb(0,0,50)`;
    return `rgb(${r}, ${g}, ${b})`;
}

let perlinNoise = new Noise();
const STEP = 0.01;
function makeNoiseMap(){
    let ret = {};
    for(let i=0; i<1000*STEP; i+=STEP) for(let j=0; j<1000*STEP; j+=STEP){
        ret[i+","+j] = perlinNoise.noiseWithOctave(i, j, 1, 3, 0.75);
    }
    return ret;
}
function draw_2d(){
    let canvas = document.getElementById("canv");
    let ctx = canvas.getContext('2d');

    let res = makeNoiseMap();
    let mx = -1e9, mn = 1e9;
    for(let i=0; i<1000*STEP; i+=STEP) for(let j=0; j<1000*STEP; j+=STEP){
        let now = res[i+","+j]; mx = Math.max(mx, now); mn = Math.min(mn, now);

        ctx.fillStyle = get_color(now, GRAY);
        ctx.fillRect(i/STEP, j/STEP, 1, 1);

        ctx.fillStyle = get_color(now, COLOR);
        ctx.fillRect(i/STEP, j/STEP+1010, 1, 1);

        ctx.fillStyle = get_color(now, HIGHLIGHT);
        ctx.fillRect(i/STEP, j/STEP+2020, 1, 1);
    }
    console.log(mx);
    console.log(mn);

    //draw_vector();
}

/*function canvas_arrow(context, x1, y1, x2, y2) {
    let len = 10; // length of head in pixels
    let dx = x2 - x1;
    let dy = y2 - y1;
    let angle = Math.atan2(dy, dx);
    context.beginPath();
    context.moveTo(x1, y1);
    context.lineTo(x2, y2);
    context.lineTo(x2 - len * Math.cos(angle - Math.PI / 6), y2 - len * Math.sin(angle - Math.PI / 6));
    context.moveTo(x2, y2);
    context.lineTo(x2 - len * Math.cos(angle + Math.PI / 6), y2 - len * Math.sin(angle + Math.PI / 6));
    context.stroke();
}

function draw_vector(){
    let canvas = document.getElementById("canv");
    let ctx = canvas.getContext('2d');

    for(let i=1; i<10; i++) for(let j=1; j<10; j++){
        let vec = p._gradient_vector_2d(i, j);
        let x = vec.x, y = vec.y;
        ctx.strokeStyle = "rgb(255, 0, 0)";
        canvas_arrow(ctx, i*100, j*100, (i+x*0.8)*100, (j+y*0.8)*100);
    }
}

function draw_1d(){
    let canvas = document.getElementById("canv");
    let ctx = canvas.getContext('2d');

    for(let i=0; i<10; i+=0.01){
        let t = (p.get(i, 3.3) + 1) / 2.0 * 1000;
        ctx.fillStyle = get_color(p.get(i, 2.06));
        ctx.fillRect(i*100, 1000-t, 1, t);
    }
}*/