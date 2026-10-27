# AutoDecorator

Geode mod for Geometry Dash 2.2081 (Android64, arm64-v8a), Geode SDK 5.10.1.
Work in progress: Stage 2 (editor interface).

## Repo layout

All files sit in the top level of the repo, except the workflow:

    .github/workflows/build.yml
    CMakeLists.txt
    mod.json
    about.md
    README.md
    main.cpp
    DecoCore.hpp
    DecoCoreImpl.cpp
    EditorBridge.hpp
    EditorBridgeImpl.cpp
    DecoPopup.hpp
    DecoPopupImpl.cpp
    EditorPauseLayerHook.cpp

## Build from a phone (no PC)

1. Upload every file except build.yml (Add file > Upload files).
2. Put build.yml at `.github/workflows/build.yml` LAST, so the build runs once with everything present.
3. Open the Actions tab and tap the newest run.
4. Green check: scroll to Artifacts and tap `AutoDecorator-android64`.
5. Unzip it, then copy the `.geode` file to
   `/storage/emulated/0/Android/media/com.geode.launcher/game/geode/mods/`
6. Start Geometry Dash through the Geode launcher and check the Geode mods list.

If the run fails, open the failed step and copy the red error lines.
