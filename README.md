# Game Feature Extension

## <a id="head"></a> 📋 〢 Overview

A plugin that extends the existing ``GameFeatures`` plugin by adding common GameFeatureActions and other goodies to the editor.  
Also with some inspiration from the [Lyra Starter Game](https://github.com/johnlogostini/Lyra)

---

## 🌐 〢 Index
> - 📣 [__Actions__]()<br>
> &nbsp; ・ [World Action Base](#actions_worldactionbase)<br>
> &nbsp; ・ [Add Input Mapping Context](#actions_addinputmappingcontext)<br>
> - 🔗 [Game Feature Plugin URL](#gfpurl)<br>
> - 📑 [Game Feature Action Set](#GameFeatureActionSet)<br>

---

### <a name="actions_worldactionbase"></a>📣 〢 World Action Base

``UGameFeatureAction_WorldActionBase``  
  
Base class for GameFeatureActions that affect the world in some way.<br>
For example, adding input bindings, setting up player controllers, etc.<br>

```cpp
/** Subclasses should override this to add their world-specific functionality */
virtual void OnAddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
```

---

### <a id="actions_addinputmappingcontext"></a>📣 〢 Add Input Mapping Context

``UGameFeatureAction_AddInputMappingContext``  _inherits_ [UGameFeatureAction_WorldActionBase](#actions_worldactionbase)  
  
GameFeatureAction responsible for adding Input Mapping Contexts to a local player's EnhancedInput system.<br>
Expects the local player has set up their EnhancedInput system.

<img src="https://github.com/user-attachments/assets/0f1d35d4-2893-4c9c-98f9-384bbcdb5772" alt="UGameFeatureAction_AddInputMappingContext" width="100%"/>

---

### <a id="gfpurl"></a>🔗 〢 Game Feature Plugin URL

``FGameFeaturePluginURL``  
  
A struct that contains a string reference to a game feature plugin.<br>
This can be used to reference game feature plugins that are loaded on demand.

```cpp
// Example usage

/** List of Game Feature Plugins this experience depends on */
UPROPERTY(EditDefaultsOnly, Category = "Dependencies")
TArray<FGameFeaturePluginURL> GameFeaturesToEnable;
```

<img src="https://github.com/user-attachments/assets/9d7ef94d-202f-4256-a01d-733112c56ed5" alt="FGameFeaturePluginURL" width="100%"/>

---

### <a id="GameFeatureActionSet"></a> 📑 〢 Game Feature Action Set

Coming soon...

---

<p align="center">
If you like this project, leaving a star is much appreciated<br>
<a href="#head">
Back to the top
</a>
</p>
