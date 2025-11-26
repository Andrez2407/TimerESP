

  const timerDisplay = document.getElementById('timer-display');
  const controlBtn = document.getElementById('control-btn');
  const lapsList = document.getElementById('laps-list');
  let timerInterval = null;
  let seconds = 0;
    // Variables de control del temporizador
    let startTime = 0;      // Tiempo en el que se inició el temporizador (timestamp)
    let elapsedTime = 0;    // Tiempo total transcurrido (en milisegundos)
    let elapsedTimeSector = 0;    // Tiempo total transcurrido (en milisegundos)
    let clickCount = 0;     // Contador de clics: 0=detenido/listo, 1=iniciado, 2=vuelta1, 3=vuelta2, 4=detener



// Array para guardar los tiempos intermedios
var currentPlayer = "Prueba";
const laps = [];


let p1 = 999999900;
let p2 = 999999900;
let p3 = 999999900;
let p4 = 999999900;
let b1 = 999999900;
let b2 = 999999900;
let b3 = 999999900;
let b4 = 999999900;
let nameb1 = "";
let nameb2 = "";
let nameb3 = "";
let nameb4 = "";



let editando = false;

// Elementos del DOM

/**
 * Función que formatea milisegundos a MM:SS:mmm
 * @param {number} ms - Milisegundos a formatear
 * @returns {string} - Cadena de tiempo formateada
 */
function formatTime(ms) {
    const totalSeconds = Math.floor(ms / 1000);
    const minutes = Math.floor(totalSeconds / 60).toString().padStart(2, '0');
    const seconds = (totalSeconds % 60).toString().padStart(2, '0');
    const milliseconds = (ms % 1000).toString().padStart(3, '0');
    return `${minutes}:${seconds}:${milliseconds}`;
}

/**
 * Función principal para actualizar el display del temporizador.
 * Se llama cada 10 milisegundos.
 */
function updateTimer() {
    elapsedTime = Date.now() - startTime;
    elapsedTimeSector = Date.now() - startTime;
    timerDisplay.textContent = formatTime(elapsedTime);
}

/**
 * Función para iniciar o reanudar el temporizador.
 */
function startTimer() {
    // Si el temporizador no está corriendo, lo iniciamos
    if (!timerInterval) {
        // Establece el tiempo de inicio ajustado por el tiempo que ya ha pasado
        startTime = Date.now() - elapsedTime;
        timerInterval = setInterval(updateTimer, 10); // Actualiza cada 10ms

        // Actualiza el botón y contador
        controlBtn.textContent = 'Guardar Vuelta 1';
        controlBtn.className = 'lap';
        clickCount = 1;

          if(b1 < 9999900){
          document.getElementById('tiempo-best').innerHTML = formatTime(b1);
          document.getElementById('name-best').textContent = nameb1;
          }else{
            document.getElementById('tiempo-best').innerHTML = formatTime(0);
          }


    }
}

/**
 * Función para guardar el tiempo actual como una 'vuelta' o 'lap'.
 */
function saveLap(lapNumber) {
    const timeToSave = formatTime(elapsedTime);
    laps.push({ number: lapNumber, time: elapsedTime });
    
    // Muestra el tiempo en la lista
    const listItem = document.createElement('li');
    listItem.textContent = `Tiempo ${lapNumber}: ${timeToSave}`;
    lapsList.appendChild(listItem);

    // Actualiza el botón
    if (lapNumber === 1) {
        controlBtn.textContent = 'Guardar Vuelta 2';
        clickCount = 2;

        if (elapsedTimeSector <= b1) { // Si el tiempo es mayor o igual a 15 segundos
                    document.getElementById('s1-text').style.color = 'purple';
                    document.getElementById('s1-label').style.backgroundColor = 'purple';
        }else if(elapsedTimeSector <= p1){
                    document.getElementById('s1-text').style.color = 'green';
                    document.getElementById('s1-label').style.backgroundColor = 'green';
        }else{
                    document.getElementById('s1-text').style.color = 'yellow';
                    document.getElementById('s1-label').style.backgroundColor = 'yellow';
        }

        if(b2 < 9999900){
          document.getElementById('tiempo-best').innerHTML = formatTime(b2);
          document.getElementById('name-best').textContent = nameb2;
        }else{
            document.getElementById('tiempo-best').innerHTML = formatTime(0);
          }


    } else if (lapNumber === 2) {
        controlBtn.textContent = 'Detener';
        controlBtn.className = 'stop';
        clickCount = 3;
        if (elapsedTimeSector <= b2) { // Si el tiempo es mayor o igual a 15 segundos
                    document.getElementById('s2-text').style.color = 'purple';
                    document.getElementById('s2-label').style.backgroundColor = 'purple';
        }else if(elapsedTimeSector <= p2){
                    document.getElementById('s2-text').style.color = 'green';
                    document.getElementById('s2-label').style.backgroundColor = 'green';
        }else{
                    document.getElementById('s2-text').style.color = 'yellow';
                    document.getElementById('s2-label').style.backgroundColor = 'yellow';
        }

        if(b3 < 9999900){
          document.getElementById('tiempo-best').innerHTML = formatTime(b3);
          document.getElementById('name-best').textContent = nameb3;
        }else{
            document.getElementById('tiempo-best').innerHTML = formatTime(0);
          }


    }

    elapsedTimeSector = 0;
}

/**
 * Función para detener el temporizador y registrar el tiempo final.
 */
function stopTimer() {
    // Detiene el intervalo
    clearInterval(timerInterval);
    timerInterval = null;

    // Guarda el tiempo final (Tiempo 3 o Total)
    const finalTime = formatTime(elapsedTime);
    laps.push({ number: 'Final', time: elapsedTime });
    
    const listItem = document.createElement('li');
    listItem.textContent = `Tiempo Final: ${finalTime}`;
    lapsList.appendChild(listItem);
    
    // Resetea el botón para un nuevo inicio
    controlBtn.textContent = 'Reiniciar';
    controlBtn.className = 'start';
    clickCount = 4; // Marcamos que ha terminado

        if (elapsedTimeSector <= b3) { // Si el tiempo es mayor o igual a 15 segundos
                    document.getElementById('s3-text').style.color = 'purple';
                    document.getElementById('s3-label').style.backgroundColor = 'purple';
        }else if(elapsedTimeSector <= p3){
                    document.getElementById('s3-text').style.color = 'green';
                    document.getElementById('s3-label').style.backgroundColor = 'green';
        }else{
                    document.getElementById('s3-text').style.color = 'yellow';
                    document.getElementById('s3-label').style.backgroundColor = 'yellow';
        }

    guardarTiempoCarrera("Prueba", laps[0].time, laps[1].time, laps[2].time);

}

/**
 * Función para resetear el temporizador a su estado inicial.
 */
function resetTimer() {
    stopTimer(); // Asegurarse de que esté detenido
    elapsedTime = 0;
    elapsedTimeSector = 0;
    laps.length = 0;
    timerDisplay.textContent = formatTime(0);
    lapsList.innerHTML = ''; // Limpiar la lista de laps

    document.getElementById('s1-text').style.color = 'white';
    document.getElementById('s1-label').style.backgroundColor = 'white';
    document.getElementById('s2-text').style.color = 'white';
    document.getElementById('s2-label').style.backgroundColor = 'white';
    document.getElementById('s3-text').style.color = 'white';
    document.getElementById('s3-label').style.backgroundColor = 'white';

    // Restablecer el botón
    controlBtn.textContent = 'Iniciar';
    controlBtn.className = 'start';
    clickCount = 0;
}


// Manejador de eventos principal
controlBtn.addEventListener('click', () => {
    switch (clickCount) {
        case 0:
            // 1er Clic: Iniciar el temporizador
            startTimer();
            break;
        case 1:
            // 2do Clic: Guardar Tiempo 1 (Vuelta 1)
            saveLap(1);
            break;
        case 2:
            // 3er Clic: Guardar Tiempo 2 (Vuelta 2)
            saveLap(2);
            break;
        case 3:
            // 4to Clic: Detener y Guardar Tiempo Final
            stopTimer();
            break;
        case 4:
            // 5to Clic o más: Reiniciar
            resetTimer();
            break;
        default:
            console.error("Estado de clickCount inesperado");
    }
});


  // 1. Inicializar la conexión WebSocket
  // ws://192.168.4.1:81  (ws:// + IP + : + Puerto WebSocket)
  const ipAddress = window.location.hostname; // Obtiene 192.168.4.1 automáticamente
  let ws = new WebSocket("ws://" + ipAddress + ":81"); 

  // Función para iniciar el temporizador (la misma que antes)

  // 2. Manejar mensajes entrantes (la 'señal' del ESP32)
  ws.onmessage = function(event) {
    console.log("Mensaje recibido: " + event.data);
    
    // Si el ESP32 envía el mensaje 'START_TIMER', iniciamos el temporizador
    if (event.data === 'START_TIMER') {
      //startTimer();
      console.log("Se recibió la señal para iniciar el temporizador.");
    }


    if(event.data === 'S1'){
        if(clickCount === 0){
            startTimer();
        }else if(clickCount === 3){
            stopTimer();
        }
    }

    if(event.data === 'S2'){
        if(clickCount === 1){
            saveLap(1);
        }
    }

    if(event.data === 'S3'){
        if(clickCount === 2){
            saveLap(2);
        }
    }


  };

  // 3. Manejar errores y cierre de conexión (opcional, pero recomendado)
  ws.onclose = function(event) {
    console.log('Conexión WebSocket cerrada. Código: ' + event.code);
  };
  ws.onerror = function(error) {
    console.log('Error de WebSocket: ' + error.message);
  };
  ws.onopen = function(event) {
    console.log('Conexión WebSocket establecida con éxito.');
  };

 


  

async function cargarDatos() {
  try {
    const res = await fetch('/api/get');
    const datos = await res.json();

    if (!Array.isArray(datos)) {
      console.warn("Respuesta inesperada de /api/get:", datos);
      mostrarTabla([]);
      return;
    }

    mostrarTabla(datos);
    obtenerRecord(datos, currentPlayer);
  } catch (err) {
    console.error("Error al cargar datos:", err);
    mostrarTabla([]);
  }
}

function mostrarTabla(datos) {
  const tbody = document.getElementById('tablaDatos');
  if (!tbody) return;
  tbody.innerHTML = '';

  if (!Array.isArray(datos) || datos.length === 0) {
    tbody.innerHTML = '<tr><td colspan="5">Sin datos disponibles</td></tr>';
    return;
  }

  datos.forEach(t => {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td>${t.nombre}</td>
      <td>${t.s1}</td>
      <td>${t.s2}</td>
      <td>${t.s3}</td>
      <td class="actions">
        <button class="edit" onclick="editar('${t.nombre}', ${t.s1}, ${t.s2}, ${t.s3})">Editar</button>
        <button onclick="eliminar('${t.nombre}')">Eliminar</button>
      </td>
    `;
    tbody.appendChild(tr);
  });
}

  function obtenerRecord(datos, jugador) {

    datos.forEach(t => {

    if(t.nombre == jugador){
        if(p1 > t.s1){ p1 = t.s1}
        if(p2 > t.s1){ p2 = t.s2}
        if(p3 > t.s1){ p3 = t.s3}
    }

    if(b1 > t.s1){ 
        b1 = t.s1
        nameb1 = t.nombre
    }
    if(b2 > t.s1){
        b2 = t.s2
        nameb2 = t.nombre
        }

    if(b3 > t.s1){ 
        b3 = t.s3
        nameb3 = t.nombre
    }


  });

  console.log("P1: " + p1 + " P2: " + p2 + " P3: " + p3);
  console.log("B1: " + b1 + " B2: " + b2 + " B3: " + b3);

  }


    async function guardarTiempo(e) {
      e.preventDefault();
      const nombre = document.getElementById('nombre').value.trim();
      const s1 = parseFloat(document.getElementById('s1').value);
      const s2 = parseFloat(document.getElementById('s2').value);
      const s3 = parseFloat(document.getElementById('s3').value);

      if (!nombre) return alert('Nombre obligatorio');

      const obj = { nombre, s1, s2, s3 };
      const url = editando ? '/api/edit' : '/api/add';

      await fetch(url, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(obj)
      });

      limpiarFormulario();
      cargarDatos();
    }

    function editar(nombre, s1, s2, s3) {
      document.getElementById('nombre').value = nombre;
      document.getElementById('s1').value = s1;
      document.getElementById('s2').value = s2;
      document.getElementById('s3').value = s3;
      document.getElementById('nombre').readOnly = true;
      document.getElementById('btnGuardar').textContent = 'Guardar cambios';
      editando = true;
    }

    async function eliminar(nombre) {
      if (!confirm(`¿Eliminar el tiempo "${nombre}"?`)) return;
      await fetch(`/api/delete?nombre=${encodeURIComponent(nombre)}`);
      cargarDatos();
    }

    cargarDatos();


   export async function guardarTiempoCarrera(nombre, t1, t2, t3) {
      if (!nombre) return alert('Nombre obligatorio');

        const s1 = t1;
        const s2 = t2 - t1;
        const s3 = t3 - (t2);
        const final = s1 + s2 + s3;

      const obj = { nombre, s1, s2, s3 , final};
      const url = editando ? '/api/edit' : '/api/add';

      await fetch(url, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(obj)
      });

      cargarDatos();
    }
