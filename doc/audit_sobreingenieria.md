# Auditoría de sobre-ingeniería — OpenGL Modeler

Análisis completo del árbol de fuentes buscando abstracciones innecesarias, código muerto y reimplementaciones de la biblioteca estándar. Los hallazgos están ordenados de mayor a menor impacto (líneas eliminables).

---

---

## 2. `AssetRegistry` → inline en `AssetManager`

**Tag:** `delete`  
**Archivos:** `src/AssetManager/AssetRegistry.hpp/.cpp`

`AssetRegistry` es una clase que envuelve un único `std::map<AssetHandle, std::shared_ptr<IAsset>>`. El único caller es `AssetManager`. No hay ninguna razón para que exista como clase separada: es un nivel de indirección sin beneficio.

**Qué hacer:** mover el `map` y sus métodos de acceso como miembros privados de `AssetManager`. Eliminar los dos archivos.

**Ganancia estimada:** −~60 líneas, −1 archivo de cabecera, −1 nivel de indirección.

---

## 3. `IAssetLoader` + `LoaderSystem` → mapa de funciones

**Tag:** `yagni`  
**Archivos:** `src/AssetManager/Loader/IAssetLoader.hpp`, `LoaderSystem.hpp/.cpp`

Existen exactamente **dos** loaders concretos: `TextureLoader` y `MeshLoader`. La lógica de despacho es trivial: extraer la extensión del archivo y buscarla en un mapa. `LoaderSystem` es una clase de 16 líneas que envuelve ese mapa con una interfaz abstracta `IAssetLoader`.

**Qué hacer:** eliminar la interfaz y el sistema de despacho. En `AssetManager::Init()` declarar directamente:

```cpp
std::map<std::string, std::function<std::shared_ptr<IAsset>(std::string)>> loaders {
    {".obj",  [](auto p){ return MeshLoader{}.Load(p); }},
    {".png",  [](auto p){ return TextureLoader{}.Load(p); }},
    {".jpg",  [](auto p){ return TextureLoader{}.Load(p); }},
};
```

**Ganancia estimada:** −~80 líneas, −3 archivos, −1 abstracción.

---

## 4. `InputManager` legacy → eliminar

**Tag:** `delete`  
**Archivos:** `src/Platform/System/Input.hpp/.cpp` (copia antigua)

El propio comentario en el código dice "Legacy GLFW input singleton (InputManager); superseded by Platform/System/Input/Input.hpp". La nueva implementación `Input` + `GlfwInput` ya existe y es el sistema activo. El singleton viejo sigue presente sin ningún caller activo.

**Qué hacer:** eliminar los archivos legacy. El nuevo sistema ya cubre todo.

**Ganancia estimada:** −~80 líneas, −2 archivos, −ambigüedad de qué input usar.

---

## 5. `Window` abstract base → eliminar

**Tag:** `yagni`  
**Archivos:** `src/Platform/System/Window/Window.hpp`

`Window` es una interfaz abstracta con una sola implementación concreta: `GlfwWindow`. `Application` construye siempre un `GlfwWindow` y nunca intercambia implementaciones. La abstracción no añade nada.

**Qué hacer:** usar `GlfwWindow` directamente en `Application`. Eliminar `Window.hpp`.

**Ganancia estimada:** −~30 líneas, −1 archivo, −1 vtable innecesaria.

---

## 6. `ITask` + fase `Execute()` vacía → `std::function`

**Tag:** `yagni`  
**Archivos:** `src/Platform/Jobs/Task.hpp`, `TaskQueue.hpp/.cpp`

`ITask` tiene dos métodos virtuales: `Execute()` (pensado para trabajo de CPU) y `Finalize()` (para trabajo de GPU/hilo principal). El único tipo concreto que existe es `AssetLoadTask`, cuyo `Execute()` es literalmente `{}` — vacío. Todo el trabajo ocurre en `Finalize()`.

El diseño contempla una separación CPU/GPU que nunca se materializa.

**Qué hacer:** eliminar la abstracción. `TaskQueue` puede contener `std::vector<std::function<void()>>` con las tareas de finalización:

```cpp
void TaskQueue::Submit(std::function<void()> finalize) {
    m_Pending.push_back(std::move(finalize));
}
void TaskQueue::ProcessCompletions() {
    for (auto& f : m_Pending) f();
    m_Pending.clear();
}
```

**Ganancia estimada:** −~50 líneas, −1 interfaz abstracta.

---

## 7. `TaskHandle` / `TaskStatus` / polling API → eliminar

**Tag:** `delete`  
**Archivos:** `src/Platform/Jobs/Task.hpp`, `TaskQueue.hpp`

`TaskStatus` (Pending, Running, Completed, Failed), `TaskHandle` (uint64_t), `GetStatus()` e `IsComplete()` están definidos pero **ningún caller los invoca**. La finalización se señaliza por callback; nadie hace polling.

**Qué hacer:** eliminar todo el API de polling. `Submit()` puede devolver `void`.

**Ganancia estimada:** −~25 líneas.

---

## 8. `LayerStack` overlay logic → `push_back` simple

**Tag:** `yagni`  
**Archivos:** `src/Platform/Layers/LayerStack.hpp/.cpp`

`LayerStack` mantiene `m_LayerInsertIndex` para diferenciar "capas regulares" (insertadas antes del índice) de "overlays" (insertadas después). En la práctica, solo existen **dos capas** y ambas son regulares. El mecanismo de overlay nunca se usa.

**Qué hacer:** reemplazar la lógica de inserción posicional por `push_back` / `erase` simples. El índice y toda la lógica de bifurcación sobran.

**Ganancia estimada:** −~20 líneas.

---

## 9. `Panel` abstract base → struct con `std::function`

**Tag:** `yagni`  
**Archivos:** `src/Editor/Panels/Panel.hpp`, todos los archivos de panel

`Panel` define un único método virtual: `OnImGuiRender()`. Los cuatro paneles concretos lo sobreescriben. Sin embargo, `EditorLayer` crea cada panel **por tipo concreto** y los llama directamente — nunca los trata polimórficamente a través del puntero base.

**Qué hacer:**

```cpp
struct PanelEntry { std::function<void()> render; bool visible = true; };
std::vector<PanelEntry> m_Panels;
```

Cada lambda captura lo que necesite. Más fácil de añadir paneles, menos archivos.

**Ganancia estimada:** −~50 líneas, −1 clase base, −vtable por panel.

---

## 10. `MainMenuBar` hereda de `Panel` innecesariamente

**Tag:** `delete`  
**Archivos:** `src/Editor/Panels/MainMenuBar.hpp/.cpp`

`MainMenuBar` hereda de `Panel` solo para tener `OnImGuiRender()`. No usa `Panel::IsVisible`, no se almacena en el vector de paneles de `EditorLayer` y se llama de forma separada. La herencia no aporta nada.

**Qué hacer:** convertirlo en una clase independiente sin herencia.

**Ganancia estimada:** −5 líneas, +claridad.

---

## 11. `IAsset` base class → Handle directo en cada tipo

**Tag:** `yagni`  
**Archivos:** `src/Platform/CoreUtils/IAsset.hpp`

`IAsset` tiene exactamente dos miembros: `virtual ~IAsset()` y `AssetHandle Handle`. No hay métodos virtuales de comportamiento. `Mesh` y `Texture` heredan de ella sin añadir nada polimórfico.

**Qué hacer:** eliminar la clase base. Añadir `AssetHandle Handle` directamente a `Mesh` y `Texture`. El registro puede usar `std::variant<shared_ptr<Mesh>, shared_ptr<Texture>>` o simplemente buscar por tipo en el mapa.

**Ganancia estimada:** −~20 líneas, −1 archivo.

---

## 12. `System::OnAttach()` + `HistorySystem::OnUpdate()` vacío

**Tag:** `shrink`  
**Archivos:** `src/Core/Systems/System.hpp`, `HistorySystem.hpp`

`System::OnAttach(Scene*)` solo asigna `m_Scene = scene`. Cuatro sistemas lo heredan sin sobreescribirlo. `HistorySystem::OnUpdate(float)` es `{}`: se llama cada frame pero no hace nada. `HistorySystem` solo se activa mediante `Push`/`Undo`/`Redo` desde la UI.

**Qué hacer:** pasar `Scene*` en el constructor de cada sistema. Eliminar `OnAttach()`. Hacer `OnUpdate` opcional (no pura virtual) o eliminar el override vacío.

**Ganancia estimada:** −~15 líneas.

---

## 13. `struct Component {}` tag vacío

**Tag:** `delete`  
**Archivos:** `src/Core/Components/Component.hpp` (la struct base vacía)

`struct Component {}` es un tag sin miembros ni métodos. EnTT no requiere clase base para los componentes — cualquier tipo es válido. El tag solo añade ruido de herencia sin valor en tiempo de compilación ni de ejecución.

**Qué hacer:** eliminar la struct base. Quitar el `: public Component` de cada componente concreto.

**Ganancia estimada:** −~10 líneas.

---

## Resumen

| Hallazgo | Líneas aprox. | Archivos |
|---|---|---|
| AssetRegistry inline | −60 | −2 |
| IAssetLoader + LoaderSystem | −80 | −3 |
| InputManager legacy | −80 | −2 |
| Window abstract base | −30 | −1 |
| ITask + Execute vacío | −50 | − |
| TaskHandle/TaskStatus polling | −25 | − |
| LayerStack overlay logic | −20 | − |
| Panel abstract base | −50 | − |
| MainMenuBar herencia | −5 | − |
| IAsset base class | −20 | −1 |
| System::OnAttach + OnUpdate vacío | −15 | − |
| Component tag vacío | −10 | − |
| **Total** | **~−635** | **~−14** |

**Prioridad:** AssetRegistry inline da el mayor retorno por esfuerzo. El resto son eliminaciones de ruido que pueden hacerse de forma incremental.
