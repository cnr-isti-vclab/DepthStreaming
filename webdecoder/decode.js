let gl, ibo, vbo, shader;

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

    let vertSrc = `
        precision highp float;

        attribute vec2 a_Position;
        attribute vec2 a_TexCoords;

        varying vec2 v_FragPos;
        varying vec2 v_TexCoords;

        void main()
        {
            gl_Position = vec4(a_Position, 0.0, 1.0);
            v_FragPos = a_Position;
            v_TexCoords = a_TexCoords;
        }
    `;

    let fragSrc = `
        precision highp float;

        varying vec3 v_FragPos;
        varying vec3 v_TexCoords;

        void main()
        {
            gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
        }
    `;

    // Create the shader
    shader = {
        handle: gl.createProgram(),
        posIdx:  0,
        texIdx: 1
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

    // Quad data
    let vertices = [-1, -1, 1, -1, 1, 1, -1, 1];
    let indices = [0, 2, 1, 0, 3, 2];

    // Create vbo, send data
    vbo = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
    getGLError();

    // Create ibo, send data
    ibo = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(indices), gl.STATIC_DRAW);
    getGLError();

    draw();    
}

function draw() {
    console.log("Drawing");

    gl.useProgram(shader.handle);

    // Bind buffers
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
    gl.enableVertexAttribArray(shader.posIdx);
    gl.vertexAttribPointer(shader.posIdx, 2, gl.FLOAT, false, 0, 0);

    // Bind indices and draw
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);
    gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);
    getGLError();

    requestAnimationFrame(draw);
}