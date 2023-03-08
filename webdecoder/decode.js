let gl;
let ibo, vertBuffer, texBuffer;
let shader, textures = {};

let currAlgo = "hilbert", currAlgoInt;
let needsUpdate = true;
let btnIds = ["hilbert-btn", "packed-btn", "split-btn", "hue-btn", "phase-btn", "triangle-btn", "morton-btn" ];
let algoIdxs = {"hilbert": 0, "triangle":1, "morton":2, "packed":3, "split":4, "phase":5, "hue": 6};
let quantization = 16;
let algoBits = 8;

function getGLError() {
    let error = gl.getError();
    if (error == gl.NO_ERROR) {
        return false;
    }

    switch (error) {
        case gl.INVALID_ENUM:
            console.error("Invalid enum");
            break;
        case gl.INVALID_VALUE:
            console.error("Invalid value");
            break;
        case gl.INVALID_OPERATION:
            console.error("Invalid operation");
            break;
        case gl.INVALID_FRAMEBUFFER_OPERATION:
            console.error("Invalid frame buffer operation");
            break;
    }

    console.trace();
}
window.onload = function() {
    let canvas = document.getElementById("gl-canvas");
    gl = canvas.getContext('webgl2');

    setupButtons();

    createShader();
    generateTextures();
    setupBuffers();

    draw();    
}

function draw() {
    if (needsUpdate) {
        if (currAlgo === "hilbert") algoBits = 5;
        else algoBits = 8;

        gl.useProgram(shader.handle);
        // Bind texture
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, textures[currAlgo])
        gl.uniform1i(shader.textureLocation, 0);
    
        // Bind buffers
        gl.bindBuffer(gl.ARRAY_BUFFER, vertBuffer);
        gl.enableVertexAttribArray(shader.posIdx);
        gl.vertexAttribPointer(shader.posIdx, 2, gl.FLOAT, false, 0, 0);

        gl.bindBuffer(gl.ARRAY_BUFFER, texBuffer);
        gl.enableVertexAttribArray(shader.texIdx);
        gl.vertexAttribPointer(shader.texIdx, 2, gl.FLOAT, false, 0, 0);
    
        // Bind indices and draw
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);
        gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);
        getGLError();
    
        needsUpdate = false;
    }
    requestAnimationFrame(draw);
}

function createShader() {
    // Create the shader
    shader = {
        handle: gl.createProgram(),
        posIdx:  0,
        texIdx: 1,
        textureLocation: -1,
        algoIdxLocation: -1
    };
    let vertShader = gl.createShader(gl.VERTEX_SHADER);
    let fragShader = gl.createShader(gl.FRAGMENT_SHADER);
    getGLError();

    // Compile
    gl.shaderSource(vertShader, vertSrc);
    gl.shaderSource(fragShader, fragSrc);
    getGLError();

    gl.compileShader(vertShader);
    gl.compileShader(fragShader);
    getGLError();

    let compiled = gl.getShaderParameter(vertShader, gl.COMPILE_STATUS);
    if (!compiled) {
        let compilationLog = gl.getShaderInfoLog(vertShader);
        console.log('Shader compiler log: ' + compilationLog);
        return;
    }

    compiled = gl.getShaderParameter(fragShader, gl.COMPILE_STATUS);
    if (!compiled) {
        let compilationLog = gl.getShaderInfoLog(fragShader);
        console.log('Shader compiler log: ' + compilationLog);
        return;
    }
    
    // Link
    gl.attachShader(shader.handle, vertShader)
    gl.attachShader(shader.handle, fragShader);
    getGLError();

    gl.bindAttribLocation(shader.handle, shader.posIdx, "a_Position");
    getGLError();
    gl.bindAttribLocation(shader.handle, shader.texIdx, "a_TexCoords");
    getGLError();

    gl.linkProgram(shader.handle);
    let linked = gl.getProgramParameter(shader.handle, gl.LINK_STATUS);
    if (!linked) {
        let linkLog = gl.getProgramInfoLog(shader.handle);
        console.log("Shader linking log: " + linkLog);
        return;
    }

    shader.textureLocation = gl.getUniformLocation(shader.handle, "u_EncodedTexture");
    shader.algoIdxLocation = gl.getUniformLocation(shader.handle, "u_AlgoIdx");
}

function generateTextures() {
    let imgs = document.getElementsByClassName("encoded-texture");
    var canvas = document.createElement('canvas');
    var context = canvas.getContext('2d');


    for (let i=0; i<imgs.length; i++) {
        let algoName = imgs[i].src.substring(imgs[i].src.lastIndexOf("/")+1, imgs[i].src.lastIndexOf("."));

        canvas.width = imgs[i].naturalWidth;
        canvas.height = imgs[i].naturalHeight;
        context.drawImage(imgs[i], 0, 0 );

        let imgdata = context.getImageData(0, 0, imgs[i].naturalWidth, imgs[i].naturalHeight).data;
        let tex = gl.createTexture();

        gl.bindTexture(gl.TEXTURE_2D, tex);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, imgs[i].naturalWidth, imgs[i].naturalHeight, 0, gl.RGBA, 
            gl.UNSIGNED_BYTE, new Uint8Array(imgdata));
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);

        gl.bindTexture(gl.TEXTURE_2D, null);
        textures[algoName] = tex;
    }
}

function setupBuffers() {
    // Quad data
    let vertices = [-1, -1, 1, -1, 1, 1, -1, 1];
    let texCoords = [0, 1, 1, 1, 1, 0, 0, 0];
    let indices = [0, 2, 1, 0, 3, 2];

    // Create vertex buffer, send data
    vertBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vertBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
    getGLError();

    // Same for texture coordinates
    texBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, texBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(texCoords), gl.STATIC_DRAW);
    getGLError();

    // Create ibo, send data
    ibo = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(indices), gl.STATIC_DRAW);
    getGLError();
}

function setupButtons() {
    // Setup buttons
    for (let i=0; i<btnIds.length; i++) {
        document.getElementById(btnIds[i]).onclick = function(e) {
            currAlgo = e.target.id.split("-")[0];
            needsUpdate = true;
        };
    }
    document.getElementById("decode").onclick = function() {
        needsUpdate = true;
    };
}

let vertSrc = `#version 300 es
        precision highp float;

        in vec2 a_Position;
        in vec2 a_TexCoords;

        out vec2 v_TexCoords;

        void main()
        {
            gl_Position = vec4(a_Position, 0.0, 1.0);
            v_TexCoords = a_TexCoords;
        }
    `;

    let utils = `
        int modi(int x, int y) {
            return x - y * (x / y);
        }

        int and(int a, int b) {
            int result = 0;
            int n = 1;
        
            for(int i = 0; i < 8; i++) {
                if ((modi(a, 2) == 1) && (modi(b, 2) == 1)) {
                    result += n;
                }
        
                a = a / 2;
                b = b / 2;
                n = n * 2;
        
                if(!(a > 0 && b > 0)) {
                    break;
                }
            }
            return result;
        }
    `;

    let decodeHilbert = `
        vec3 decodeHilbert(vec3 texColor) {
            return texColor;
        }
    `;

    let decodeTriangle = `
        vec3 decodeTriangle(vec3 texColor) {
            return texColor;
        }
    `;
    let decodeMorton = `
        vec3 decodeMorton(vec3 texColor) {
            return texColor;
        }
    `;
    let decodePacked = `
        vec3 decodePacked(vec3 texColor) {
            return texColor;
        }
    `;
    let decodeSplit = `
        vec3 decodeSplit(vec3 texColor) {
            return texColor;
        }
    `;
    let decodePhase = `
        vec3 decodePhase(vec3 texColor) {
            return texColor;
        }
    `;
    let decodeHue = `
        vec3 decodeHue(vec3 texColor) {
            return texColor;
        }
    `;


    let fragSrc = `#version 300 es
        precision highp float;

        uniform sampler2D u_EncodedTexture;
        uniform int u_AlgoIdx;

        in vec2 v_TexCoords;

        out vec4 FragColor;
        `

    fragSrc += utils;
    fragSrc += decodeHilbert;
    fragSrc += decodeTriangle;
    fragSrc += decodeMorton;
    fragSrc += decodePacked;
    fragSrc += decodeSplit;
    fragSrc += decodePhase;
    fragSrc += decodeHue;

    fragSrc += `
        void main()
        {
            vec3 pixColor = vec3(0.0, 0.0, 0.0);
            vec3 texColor = texture(u_EncodedTexture, v_TexCoords).xyz;

            // "hilbert": 0, "triangle":1, "morton":2, "packed":3, "split":4, "phase":5, "hue": 6
            if (u_AlgoIdx == 0)
                pixColor = decodeHilbert(texColor);
            else if (u_AlgoIdx == 1)
                pixColor = decodeTriangle(pixColor);
            else if (u_AlgoIdx == 2)
                pixColor = decodeMorton(pixColor);
            else if (u_AlgoIdx == 3)
                pixColor = decodePacked(pixColor);
            else if (u_AlgoIdx == 4)
                pixColor = decodeSplit(pixColor);
            else if (u_AlgoIdx == 5)
                pixColor = decodePhase(pixColor);
            else if (u_AlgoIdx == 6)
                pixColor = decodeHue(pixColor);

            FragColor = vec4(pixColor, 1.0);
        }
    `;