# Спецификация структуры XML-файлов проекта (CADAssembly и CADModel)

Настоящий документ описывает структуру хранения данных сцены и трехмерных геометрических сеток в формате XML, используемую в системе нарезки мешей.

Архитектура хранения разделена на два уровня:
1. **Файл Сборки / Сцены (`*.xml` / `<CADAssembly>`)** — описывает иерархию, глобальные метаданные, ссылки на внешние модели, экземпляры компонентов (инстансы), их мировые трансформации (TRS) и типы сущностей (исходные меши, секущие плоскости-слайсеры).
2. **Файл Геометрии Модели (`*.model.xml` / `<CADModel>`)** — содержит непосредственные геометрические данные сетки (массивы вершин с координатами X, Y, Z и RGBA-цветами, а также массив индексов треугольников с порядком обхода CCW).

---

## 1. Структура файла Сцены (`CADAssembly`)

Файл сцены является точкой входа при сохранении и загрузке проекта. Он не дублирует вершины моделей, а ссылается на локальные файлы геометрии в каталоге `Models/`.

### 1.1. Дерево элементов `CADAssembly`

```
CADAssembly (корень)
├── Metadata
│   ├── Generator
│   ├── Units
│   └── UpAxis
└── ComponentInstances
    ├── Instance [type="SourceModel"]
    │   ├── Transform
    │   │   ├── Position
    │   │   ├── Rotation
    │   │   └── Scale
    │   └── RenderState
    └── Instance [type="Slicer"]
        ├── Transform
        │   ├── Position
        │   ├── Rotation
        │   └── Scale
        └── RenderState
```

### 1.2. Описание тегов и атрибутов `CADAssembly`

#### Корневой элемент `<CADAssembly>`
* `version` (string) — версия схемы сборки (по умолчанию `"1.0"`).
* `name` (string) — пользовательское имя сцены.

#### Секция `<Metadata>`
* `<Generator>` (string) — название ядра/модуля, создавшего файл (`MeshCut Studio CAD Core`).
* `<Units>` (string) — единицы измерения координат (по умолчанию `Millimeters`).
* `<UpAxis>` (string) — базовая вертикальная ось пространства (`Y`).

#### Секция `<ComponentInstances>`
Содержит список всех сущностей, размещенных на сцене.
* `count` (unsigned int) — количество сохраненных компонентов в сборке.

#### Элемент `<Instance>`
Описывает конкретный экземпляр объекта в пространстве сцены.
* `id` (unsigned int) — уникальный идентификатор сущности в рамках сцены.
* `name` (string) — отображаемое имя объекта в Scene Outliner.
* `source` (string) — относительный путь к файлу геометрии модели (`Models/<Name>.model.xml`).
* `type` (string) — тип сущности:
  * `"SourceModel"` — исходный твердотельный или поверхностный 3D-меш.
  * `"Slicer"` — интерактивный инструмент секущей плоскости (нож).
* `visualSize` (float, опционально для Slicer) — физический габаритный размер плоскости отображения ножа.

#### Подчиненный тег `<Transform>`
Задает пространственное положение экземпляра:
* `<Position x="..." y="..." z="..."/>` — смещение относительно начала координат.
* `<Rotation x="..." y="..." z="..."/>` — углы поворота Эйлера в градусах (порядок: Z -> Y -> X).
* `<Scale x="..." y="..." z="..."/>` — коэффициенты масштабирования по осям.

#### Подчиненный тег `<RenderState>`
Задает параметры отрисовки:
* `visible` (bool) — видимость объекта (`true` / `false`).
* `mode` (string) — режим отображения (`"Solid"` / `"Wireframe"`).
* `color` (hex string) — шестнадцатеричный цвет модели в формате `0xRRGGBBAA`.

---

## 2. Структура файла Модели (`CADModel`)

Файл модели описывает топологию и вертексные буферы единичной сетки.

### 2.1. Дерево элементов `CADModel`

```
CADModel (корень)
├── Properties
├── Transform
└── Geometry
    ├── Vertices
    └── Triangles
```

### 2.2. Описание тегов и атрибутов `CADModel`

#### Корневой элемент `<CADModel>`
* `version` (string) — версия формата модели (`"1.0"`).
* `name` (string) — базовое наименование геометрического меша.

#### Элемент `<Properties>`
* `color` (hex string) — базовый цвет материала меша (`0xRRGGBBAA`).
* `renderType` (string) — режим рендера по умолчанию (`"Solid"` или `"Wireframe"`).
* `visible` (bool) — базовый флаг видимости.

#### Элемент `<Transform>`
Локальное смещение геометрии внутри меша (Position, Rotation, Scale).

#### Элемент `<Geometry>`
* `verticesCount` (unsigned int) — общее количество вершин.
* `trianglesCount` (unsigned int) — общее количество треугольников (равно `число индексов / 3`).

#### Текстовые блоки данных геометрии:
* `<Vertices>` — список вершин, разделенных пробелами и переносами строк. Каждая вершина описывается 4 значениями:
  ```
  X Y Z 0xRRGGBBAA
  ```
  * `X, Y, Z` (float) — локальные координаты вершины.
  * `0xRRGGBBAA` (hex) — покомпонентный цвет вершины.

* `<Triangles>` — список треугольников, представленный индексами вершин. Каждый треугольник описывается триплетом индексов:
  ```
  index0 index1 index2
  ```
  * Индексация ведется с `0`.
  * Порядок обхода индексов строго против часовой стрелки (**CCW — Counter-Clockwise**), если смотреть с внешней (лицевой) стороны полигона.

---

## 3. Шаблоны и примеры XML-файлов

### 3.1. Шаблон файла Сборки сцены (`Scene.xml`)

```xml
<?xml version="1.0" encoding="UTF-8"?>
<CADAssembly version="1.0" name="Default Assembly">
    <Metadata>
        <Generator>MeshCut Studio CAD Core</Generator>
        <Units>Millimeters</Units>
        <UpAxis>Y</UpAxis>
    </Metadata>
    <ComponentInstances count="2">
        <Instance id="0" name="Cube_Red" source="Models/Cube_Red.model.xml" type="SourceModel">
            <Transform>
                <Position x="-3.0000" y="0.0000" z="0.0000"/>
                <Rotation x="0.0000" y="0.0000" z="0.0000"/>
                <Scale x="1.0000" y="1.0000" z="1.0000"/>
            </Transform>
            <RenderState visible="true" mode="Solid" color="0xFF0000FF"/>
        </Instance>
        <Instance id="1" name="Slicer_Tool_1" source="Models/Slicer_Tool_1.model.xml" type="Slicer" visualSize="12.0000">
            <Transform>
                <Position x="0.0000" y="1.5000" z="0.0000"/>
                <Rotation x="45.0000" y="0.0000" z="0.0000"/>
                <Scale x="1.0000" y="1.0000" z="1.0000"/>
            </Transform>
            <RenderState visible="true" mode="Solid" color="0xFF005580"/>
        </Instance>
    </ComponentInstances>
</CADAssembly>
```

### 3.2. Шаблон файла 3D-Модели (`Models/Cube_Red.model.xml`)

```xml
<?xml version="1.0" encoding="UTF-8"?>
<CADModel version="1.0" name="Cube_Red">
    <Properties color="0xFF0000FF" renderType="Solid" visible="true"/>
    <Transform>
        <Position x="0.0000" y="0.0000" z="0.0000"/>
        <Rotation x="0.0000" y="0.0000" z="0.0000"/>
        <Scale x="1.0000" y="1.0000" z="1.0000"/>
    </Transform>
    <Geometry verticesCount="8" trianglesCount="12">
        <Vertices>
            -1.0000 -1.0000 -1.0000 0xFFFFFFFF
             1.0000 -1.0000 -1.0000 0xFFFFFFFF
             1.0000  1.0000 -1.0000 0xFFFFFFFF
            -1.0000  1.0000 -1.0000 0xFFFFFFFF
            -1.0000 -1.0000  1.0000 0xFFFFFFFF
             1.0000 -1.0000  1.0000 0xFFFFFFFF
             1.0000  1.0000  1.0000 0xFFFFFFFF
            -1.0000  1.0000  1.0000 0xFFFFFFFF
        </Vertices>
        <Triangles>
            4 5 6
            4 6 7
            0 2 1
            0 3 2
            0 4 7
            0 7 3
            1 2 6
            1 6 5
            3 7 6
            3 6 2
            0 1 5
            0 5 4
        </Triangles>
    </Geometry>
</CADModel>
```

### 3.3. Шаблон файла Секущей плоскости (`Models/Slicer_Tool_1.model.xml`)

```xml
<?xml version="1.0" encoding="UTF-8"?>
<CADModel version="1.0" name="Slicer_Tool_1">
    <Properties color="0xFF005580" renderType="Solid" visible="true"/>
    <Transform>
        <Position x="0.0000" y="0.0000" z="0.0000"/>
        <Rotation x="0.0000" y="0.0000" z="0.0000"/>
        <Scale x="1.0000" y="1.0000" z="1.0000"/>
    </Transform>
    <Geometry verticesCount="4" trianglesCount="4">
        <Vertices>
            -6.0000 0.0000 -6.0000 0xFFFFFFFF
             6.0000 0.0000 -6.0000 0xFFFFFFFF
             6.0000 0.0000  6.0000 0xFFFFFFFF
            -6.0000 0.0000  6.0000 0xFFFFFFFF
        </Vertices>
        <Triangles>
            0 1 2
            0 2 3
            0 2 1
            0 3 2
        </Triangles>
    </Geometry>
</CADModel>
```

---

## 4. Особенности связывания и правила сериализации

1. **Изоляция сгенерированных долек (`MeshParts`):**
   При вызове `CScene::SaveToFile` объекты типа `eOT_MeshParts` (результаты нарезки) намеренно исключаются из файла сборки. Сохраняются только сущности `SourceModel` и `Slicer`. Нарезка воспроизводится детерминированно пайплайном `ExecuteSlicingPipeline`.

2. **Относительные пути:**
   Все ссылки `source` в сборке формируются относительно базовой директории файла `Scene.xml`. Автоматически создается подкаталог `Models/`, куда выгружаются индивидуальные геометрические файлы.

3. **Санитизация имен файлов:**
   Спецсимволы (` `, `#`, `(`, `)`, `/`, `\`) в именах объектов автоматически заменяются на символ подчеркивания `_` при формировании пути на диске.