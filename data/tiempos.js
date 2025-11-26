

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

    function limpiarFormulario() {
      document.getElementById('tiempoForm').reset();
      document.getElementById('nombre').readOnly = false;
      document.getElementById('btnGuardar').textContent = 'Agregar';
      editando = false;
    }

    cargarDatos();


   export async function guardarTiempoCarrera( nombre, s1, s2, s3) {
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
