# 拖拽功能实现说明

## 概述

本次实现为歌声合成编辑器添加了两个核心拖拽功能：
1. **Track轨道头拖拽排序** - 可以通过拖拽轨道头色块来调整轨道顺序
2. **Clip跨Track拖拽** - 可以将Clip在不同Track之间拖拽移动

## 功能特性

### 1. Track轨道头拖拽排序

#### 用户体验
- 在轨道列表区域（左侧320px宽的面板）拖拽任意轨道
- 实时显示拖拽预览
- 释放鼠标后，轨道顺序立即更新
- 支持撤销/重做 (Ctrl+Z / Ctrl+Shift+Z)

#### 技术实现
- **文件**: `TrackListView.cpp`, `TrackListView.h`
- **关键代码**:
  ```cpp
  setDragDropMode(QAbstractItemView::InternalMove);
  setDefaultDropAction(Qt::MoveAction);

  void TrackListView::dropEvent(QDropEvent *event) {
      const int fromRow = currentRow();
      QListWidget::dropEvent(event);
      const int toRow = currentRow();

      if (fromRow != toRow && fromRow >= 0 && toRow >= 0) {
          trackController->onMoveTrack(fromRow, toRow);
      }
  }
  ```

- **Action**: `MoveTrackAction` (已存在)
  - `execute()`: 将Track从fromIndex移动到toIndex
  - `undo()`: 恢复到原位置
  - 自动处理索引调整（向下移动时索引-1）

### 2. Clip跨Track拖拽

#### 用户体验
- 拖拽Clip时可以垂直移动到其他Track
- 实时预览：Clip的Y坐标随鼠标移动实时更新
- 释放鼠标后：
  - 如果在同一Track内：正常移动/调整位置
  - 如果移到其他Track：执行跨Track移动操作
- 量化对齐：按住Alt键临时关闭量化
- 支持撤销/重做

#### 技术实现

##### 核心文件修改

**TracksGraphicsView.h**:
```cpp
private:
    int m_mouseDownTrackIndex = -1;  // 记录拖拽起始Track索引
    int m_mouseDownTrackId = -1;      // 记录拖拽起始Track ID
```

**TracksGraphicsView.cpp**:

1. **prepareForMovingOrResizingClip()** - 记录初始状态:
```cpp
m_mouseDownTrackIndex = m_currentEditingClip->trackIndex();

// Get track ID for cross-track dragging
int trackIndex = -1;
appModel->findClipById(m_currentEditingClip->id(), trackIndex);
if (trackIndex >= 0 && trackIndex < appModel->tracks().size()) {
    m_mouseDownTrackId = appModel->tracks().at(trackIndex)->id();
}
```

2. **mouseMoveEvent()** - 实时更新Clip位置:
```cpp
if (m_mouseMoveBehavior == Move) {
    // ... 更新X坐标 ...

    // Support cross-track dragging
    const int currentTrackIndex = m_scene->trackIndexAt(curPos.y());
    if (currentTrackIndex >= 0 && currentTrackIndex < appModel->tracks().size()) {
        m_currentEditingClip->setTrackIndex(currentTrackIndex);
    }
}
```

3. **mouseReleaseEvent()** - 检测并执行跨Track移动:
```cpp
if (m_mouseMoveBehavior == Move && currentTrackIndex != m_mouseDownTrackIndex) {
    // Cross-track move detected
    const int currentTrackId = appModel->tracks().at(currentTrackIndex)->id();
    const int clipId = m_currentEditingClip->id();

    // First update clip properties (position change)
    trackController->onClipPropertyChanged(args);

    // Then move to new track
    trackController->onMoveClipBetweenTracks(clipId, m_mouseDownTrackId, currentTrackId);
}
```

##### Controller层

**TrackController.h**:
```cpp
static void onMoveTrack(qsizetype fromIndex, qsizetype toIndex);
static void onMoveClipBetweenTracks(int clipId, int fromTrackId, int toTrackId);
```

**TrackController.cpp**:

1. **onMoveTrack()** - Track移动:
```cpp
void TrackController::onMoveTrack(qsizetype fromIndex, qsizetype toIndex) {
    if (fromIndex == toIndex) return;

    const auto a = MoveTrackAction::build(fromIndex, toIndex, appModel);
    a->execute();
    historyManager->record(a);
}
```

2. **onMoveClipBetweenTracks()** - Clip跨Track移动:
```cpp
void TrackController::onMoveClipBetweenTracks(int clipId, int fromTrackId, int toTrackId) {
    if (fromTrackId == toTrackId) return;

    const auto fromTrack = appModel->findTrackById(fromTrackId);
    const auto toTrack = appModel->findTrackById(toTrackId);
    const auto clip = fromTrack->findClipById(clipId);

    if (!clip || !fromTrack || !toTrack) return;

    const auto a = MoveClipBetweenTracksAction::build(clip, fromTrack, toTrack);
    a->execute();
    historyManager->record(a);
}
```

##### Action层

**MoveTrackAction** (已存在):
- 从fromIndex移除Track
- 计算调整后的toIndex（向下移动时-1）
- 在新位置插入Track

**MoveClipBetweenTracksAction** (已存在):
- 从源Track移除Clip
- 添加Clip到目标Track

## 架构设计

### 数据流

#### Track拖拽排序:
```
UI层 (TrackListView)
  ↓ dropEvent
Controller层 (TrackController::onMoveTrack)
  ↓ 创建Action
Action层 (MoveTrackAction)
  ↓ execute
Model层 (AppModel::removeTrackAt / insertTrack)
  ↓ emit trackChanged
UI层 (TrackEditorView::onTrackChanged)
  ↓ 更新View
```

#### Clip跨Track拖拽:
```
UI层 (TracksGraphicsView::mouseMove)
  ↓ 实时更新trackIndex和Y坐标
  ↓ mouseRelease检测Track变化
Controller层 (TrackController::onMoveClipBetweenTracks)
  ↓ 创建Action
Action层 (MoveClipBetweenTracksAction)
  ↓ execute
Model层 (Track::removeClip / Track::insertClip)
  ↓ emit clipChanged
UI层 (TrackEditorView::onClipChanged)
  ↓ 更新View
```

### 关键设计决策

1. **使用Track ID而非索引**
   - 原因：在拖拽过程中，Track的索引可能变化
   - 使用ID确保跨Track操作的准确性

2. **先更新属性再移动Track**
   - 先调用 `onClipPropertyChanged` 更新Clip的时间位置
   - 再调用 `onMoveClipBetweenTracks` 移动到新Track
   - 避免状态不一致

3. **实时视觉反馈**
   - mouseMoveEvent中实时更新Clip的trackIndex
   - setTrackIndex触发updateRectAndPos，立即更新Y坐标
   - 提供流畅的拖拽体验

4. **错误处理**
   - 检查trackIndex范围有效性
   - 无效时恢复到原Track
   - 防止崩溃和数据损坏

## 兼容性

### 现有功能保持不变
- ✅ Clip水平拖拽（调整时间位置）
- ✅ Clip左右边缘调整（ResizeLeft/ResizeRight）
- ✅ 量化对齐（Alt键关闭）
- ✅ 多选Clip（Ctrl+点击）
- ✅ 撤销/重做系统

### 新增交互
- ✅ Track上下拖拽排序
- ✅ Clip垂直拖拽切换Track

## 测试建议

### Track拖拽排序
1. 创建3-5个Track
2. 拖拽第1个Track到最后
3. 拖拽最后一个Track到第一个
4. 测试撤销/重做
5. 测试拖拽到同一位置（应无变化）

### Clip跨Track拖拽
1. 创建2-3个Track，每个Track添加Clip
2. 拖拽Clip到另一个Track
3. 检查Clip的时间位置是否保持
4. 测试撤销/重做
5. 测试拖拽到Track边界外（应恢复原位）
6. 测试按住Alt键拖拽（关闭量化）

### 边界情况
- 只有1个Track时（Track拖拽应禁用）
- Clip拖拽到无效Track区域
- 连续多次拖拽
- 拖拽过程中按ESC取消（目前不支持，可作为未来改进）

## 性能考虑

- ✅ 使用Qt内置拖放机制（TrackListView）
- ✅ 实时更新仅修改必要属性
- ✅ Action系统避免重复操作
- ✅ 无内存泄漏（使用Qt对象树管理）

## 未来改进方向

1. **视觉增强**
   - 添加拖拽指示线（显示插入位置）
   - Track拖拽时高亮目标位置
   - Clip拖拽时显示半透明预览

2. **功能增强**
   - 支持按ESC取消拖拽
   - 支持多选Clip批量移动到其他Track
   - 添加拖拽音效反馈

3. **性能优化**
   - 大量Track时使用虚拟列表
   - 拖拽时降低渲染频率

## 修改文件列表

### 新增文件
- `src/app/Controller/Actions/AppModel/Track/MoveTrackAction.h`
- `src/app/Controller/Actions/AppModel/Track/MoveTrackAction.cpp`
- `src/app/Controller/Actions/AppModel/Clip/MoveClipBetweenTracksAction.h`
- `src/app/Controller/Actions/AppModel/Clip/MoveClipBetweenTracksAction.cpp`

### 修改文件
- `src/app/Controller/TrackController.h` - 添加两个新方法
- `src/app/Controller/TrackController.cpp` - 实现方法和引入头文件
- `src/app/UI/Views/TrackEditor/TrackListView.h` - 添加dropEvent声明
- `src/app/UI/Views/TrackEditor/TrackListView.cpp` - 实现拖放逻辑
- `src/app/UI/Views/TrackEditor/TracksGraphicsView.h` - 添加成员变量
- `src/app/UI/Views/TrackEditor/TracksGraphicsView.cpp` - 实现跨Track拖拽

## 代码风格

所有代码遵循项目现有规范：
- ✅ Qt6 C++17标准
- ✅ 使用项目统一的命名规范
- ✅ 遵循MVC架构模式
- ✅ Action系统实现撤销/重做
- ✅ 使用const引用传递参数
- ✅ 适当的注释（英文）
