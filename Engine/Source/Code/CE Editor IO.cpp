﻿/******************************************************************************/
#include "stdafx.h"
namespace EE {
namespace Edit {
/******************************************************************************/
#define CC4_CDES CC4('C', 'D', 'E', 'S') // code editor symbols
#define CDES_VER 3
/******************************************************************************/
// SYMBOLS
/******************************************************************************/
Bool CodeEditor::saveSymbols(C Str &file) {
    File f;
    if (f.write(file)) {
        f.putUInt(CC4_CDES);
        f.cmpUIntV(CDES_VER); // version

        Memc<Str> strings;
        FREPA(Symbols)
        strings.add(Symbols.lockedDesc(i).file()); // add symbols to strings
        FREPA(sources)
        strings.add(sources[i].loc.file_name); // add sources to strings
        StrLibrary sl(strings, true, true);
        sl.save(f);

        f.putUInt(items.elms());
        FREPAO(items).save(f, sl);
        f.putUInt(Symbols.elms());
        FREPA(Symbols)
        sl.putStr(f, Symbols.lockedDesc(i).file());
        f.putUInt(sources.elms());
        FREPAO(sources).save(f, sl);
        f.putUInt(EEUsings.elms());
        FREPA(EEUsings)
        sl.putStr(f, EEUsings[i].name());
        f.putUInt(EEMacros.elms());
        FREPAO(EEMacros).save(f, sl);

        if (f.flushOK())
            return true;
        f.del();
        FDelFile(file);
    }
    return false;
}
/******************************************************************************/
Bool CodeEditor::loadSymbols(C Str &file, Bool all) {
    File f;
    if (f.read(file))
        if (f.getUInt() == CC4_CDES)
            if (f.decUIntV() == CDES_VER) // version
            {
                StrLibrary sl;
                if (sl.load(f)) {
                    EEUsings.del();
                    EEMacros.del();
                    // sources .del(); don't delete sources because in the Editor we can preview sources before loading symbols
                    items.del();

                    items.setNum(f.getUInt());
                    FREPAO(items).load(f, sl);
                    if (all) {
                        Str temp;
                        Int n = f.getUInt();
                        Symbols.reserve(n);
                        REP(n) {
                            sl.getStr(f, temp);
                            Symbols(temp);
                        } // create all symbols up-front in sorted order so they are created quickly (this speeds up loading time), ignore counted references because they won't be unloaded anyway, load this before sources, reserve only "n" symbols and not "Symbols.elms()+n" because some of the loaded symbols could have already been allocated
                        Int i = sources.elms();
                        sources.addNum(f.getUInt());
                        for (; i < sources.elms(); i++)
                            sources[i].load(f, sl, temp);
                        EEUsings.setNum(f.getUInt());
                        FREPAO(EEUsings).require(sl.getStr(f));
                        EEMacros.setNum(f.getUInt());
                        FREPAO(EEMacros).load(f, sl, temp);
                    }
                    Time.skipUpdate(); // this can take some time
                    return f.ok();
                }
            }
    return false;
}
/******************************************************************************/
// SETTINGS
/******************************************************************************/
void CodeEditor::saveSettings(TextNode &code) {
    TextNode &paths = code.nodes.New().setName("Paths");
    paths.nodes.New().set("ClangFormat", clang_format_path);
    paths.nodes.New().set("VisualStudio", vs_path);
    paths.nodes.New().set("NetBeans", netbeans_path);
    paths.nodes.New().set("AndroidSDK", android_sdk);
    paths.nodes.New().set("AndroidNDK", android_ndk);
    paths.nodes.New().set("JavaDevelopmentKit", jdk_path);

    TextNode &certs = code.nodes.New().setName("Certificates");
    TextNode &win_cert = certs.nodes.New().setName("Windows");
    win_cert.nodes.New().set("Use", options.authenticode());
    TextNode &android_cert = certs.nodes.New().setName("Android");
    android_cert.nodes.New().set("File", android_cert_file);
    android_cert.nodes.New().set("Password", android_cert_pass);
    TextNode &apple_cert = certs.nodes.New().setName("Apple");
    apple_cert.nodes.New().set("TeamID", apple_team_id);

    TextNode &theme = code.nodes.New().setName("Theme");
    TextNode &font = theme.nodes.New().setName("Font");
    if (InRange(options.font_size(), options.font_sizes))
        font.nodes.New().set("Size", options.font_sizes[options.font_size()].size);
    options.font_editor.params.save(font.nodes.New().setName("Custom"));
    theme.nodes.New().set("Color", options.color_theme());
    ThemeCustom.save(theme.nodes.New().setName("Custom"));

    TextNode &view = code.nodes.New().setName("View");
    view.nodes.New().set("OpenedFiles", visibleOpenedFiles());

    TextNode &edit = code.nodes.New().setName("Edit");
    edit.nodes.New().set("AutocompleteOnEnterOnly", options.ac_on_enter());
    #if WINDOWS // TODO SUPPORT MORE OS
    edit.nodes.New().set("FormatWithClangOnSave", options.clang_format_during_save());
    #endif
    edit.nodes.New().set("AutoSaveDuringWriting", options.save_during_write());
    edit.nodes.New().set("SimpleMode", options.simple());
    edit.nodes.New().set("ImmediateScroll", options.imm_scroll());
    edit.nodes.New().set("EndOfLineClip", options.eol_clip());
    edit.nodes.New().set("ShowLineNumbers", options.line_numbers());
    edit.nodes.New().set("HideHorizontalSlidebar", options.hide_horizontal_slidebar());
    edit.nodes.New().set("AutoHideMenuBar", options.auto_hide_menu());
    edit.nodes.New().set("ShowFileTabs", options.show_file_tabs());
    edit.nodes.New().set("ExportPathMode", options.export_path_mode());

    TextNode &conf = code.nodes.New().setName("Configuration");
    conf.nodes.New().set("Debug", config_debug);
    // conf.nodes.New().set("32Bit", config_32_bit);
    // conf.nodes.New().set("API"  , config_api   );
    if (auto name = ShortName(config_exe))
        conf.nodes.New().set("EXE", name);

    TextNode &sugg = code.nodes.New().setName("SuggestionHistory");
    FREPA(Suggestions)
    sugg.nodes.New().value = Suggestions[i];

    TextNode &import = code.nodes.New().setName("Importing");
    import.nodes.New().set("AssetPathMode", options.import_path_mode());
    import.nodes.New().set("ImageMipMaps", options.import_image_mip_maps());
}
/******************************************************************************/
void CodeEditor::loadSettings(C TextNode &code) {
    if (C TextNode *theme = code.findNode("Theme")) // load theme settings at start
    {
        if (C TextNode *font = theme->findNode("Font")) {
            if (C TextNode *p = font->findNode("Size")) {
                Int size = p->asInt();
                REPA(options.font_sizes)
                if (options.font_sizes[i].size == size) {
                    options.font_size.set(i);
                    break;
                }
            }
            if (C TextNode *custom = font->findNode("Custom")) {
                options.font_editor.params.load(*custom);
                REPAO(options.font_editor.props).toGui();
            }
        }

        if (C TextNode *p = theme->findNode("Color"))
            options.color_theme.set(p->asInt());
        if (C TextNode *custom = theme->findNode("Custom")) {
            ThemeCustom.load(*custom);
            REPAO(options.color_theme_editor.props).toGui();
        }
    }
    if (C TextNode *edit = code.findNode("Edit")) // editing settings
    {
        if (C TextNode *p = edit->findNode("AutocompleteOnEnterOnly"))
            options.ac_on_enter.set(p->asBool1());
        if (C TextNode *p = edit->findNode("FormatWithClangOnSave"))
            options.clang_format_during_save.set(p->asBool1());
        if (C TextNode *p = edit->findNode("AutoSaveDuringWriting"))
            options.save_during_write.set(p->asBool1());
        if (C TextNode *p = edit->findNode("SimpleMode"))
            options.simple.set(p->asBool1());
        if (C TextNode *p = edit->findNode("ImmediateScroll"))
            options.imm_scroll.set(p->asBool1());
        if (C TextNode *p = edit->findNode("EndOfLineClip"))
            options.eol_clip.set(p->asBool1());
        if (C TextNode *p = edit->findNode("ShowLineNumbers"))
            options.line_numbers.set(p->asBool1());
        if (C TextNode *p = edit->findNode("HideHorizontalSlidebar"))
            options.hide_horizontal_slidebar.set(p->asBool1());
        if (C TextNode *p = edit->findNode("AutoHideMenuBar"))
            options.auto_hide_menu.set(p->asBool1());
        if (C TextNode *p = edit->findNode("ShowFileTabs"))
            options.show_file_tabs.set(p->asBool1());
        if (C TextNode *p = edit->findNode("ExportPathMode"))
            options.export_path_mode.set(p->asBool());
    }
    if (C TextNode *paths = code.findNode("Paths")) {
        if (C TextNode *p = paths->findNode("VisualStudio"))
            setVSPath(p->value); // load VS path before loading project (and parsing 3rd party headers)
        if (C TextNode *p = paths->findNode("NetBeans"))
            setNetBeansPath(p->value);
        if (C TextNode *p = paths->findNode("AndroidSDK"))
            setAndroidSDKPath(p->value);
        if (C TextNode *p = paths->findNode("AndroidNDK"))
            setAndroidNDKPath(p->value);
        if (C TextNode *p = paths->findNode("JavaDevelopmentKit"))
            setJDKPath(p->value);
        if (C TextNode *p = paths->findNode("ClangFormat"))
            setClangFormatPath(p->value);
    }
    if (C TextNode *certificates = code.findNode("Certificates")) {
        if (C TextNode *windows = certificates->findNode("Windows")) {
            if (C TextNode *p = windows->findNode("Use"))
                options.authenticode.set(p->asBool());
        }
        if (C TextNode *android = certificates->findNode("Android")) {
            if (C TextNode *p = android->findNode("File"))
                setAndroidCertPath(p->value);
            if (C TextNode *p = android->findNode("Password"))
                setAndroidCertPass(p->value);
        }
        if (C TextNode *apple = certificates->findNode("Apple")) {
            if (C TextNode *p = apple->findNode("TeamID"))
                setAppleTeamID(p->value);
        }
    }
    if (C TextNode *proj = code.findNode("View")) {
        if (C TextNode *p = proj->findNode("OpenedFiles"))
            visibleOpenedFiles(p->asBool());
    }
    if (C TextNode *conf = code.findNode("Configuration")) {
        if (C TextNode *p = conf->findNode("Debug"))
            configDebug(p->asBool());
        // if(C TextNode *p=conf->findNode("32Bit"))config32Bit(p->asBool());
        // if(C TextNode *p=conf->findNode("API"  ))configAPI  (p->asInt ());
        if (C TextNode *p = conf->findNode("EXE")) {
            REP(EXE_NUM)
            if (p->value == ShortName(EXE_TYPE(i))) {
                configEXE(EXE_TYPE(i));
                break;
            }
        }
    }
    if (C TextNode *sugg = code.findNode("SuggestionHistory")) {
        Suggestions.clear();
        FREPA(sugg->nodes)
        Suggestions.add(sugg->nodes[i].value);
    }
    if (C TextNode *import = code.findNode("Importing")) {
        if (C TextNode *p = import->findNode("AssetPathMode"))
            options.import_path_mode.set(p->asBool());
        if (C TextNode *p = import->findNode("ImageMipMaps"))
            options.import_image_mip_maps.set(p->asBool());
    }
}
/******************************************************************************/
// SOURCES
/******************************************************************************/
static void LoadSource(C Str &name, CodeEditor &ce) { ce.load(name); }
/******************************************************************************/
Bool CodeEditor::load(C SourceLoc &loc, Bool quiet, Bool Const) {
    if (loc.is()) // if location is specified
    {
        if (loc.file && !TextExt(GetExt(loc.file_name)))
            return false; // skip files which are not text files
        ERROR_TYPE error;
        if (Source *s = getSource(loc, &error)) {
            s->Const |= Const;
            cur(s);
            return true;
        }
        if (!quiet) {
            Str msg;
            switch (error) {
            case EE_ERR_ELM_NOT_FOUND:
                msg = S + "Element \"" + loc.asText() + "\" was not found";
                break;
            case EE_ERR_ELM_NOT_CODE:
                msg = S + "Element \"" + loc.asText() + "\" is not a Code";
                break;
            case EE_ERR_ELM_NOT_DOWNLOADED:
                msg = S + "Element \"" + loc.asText() + "\" was not yet downloaded, please first:\n1. Upload Code using \"Synchronize Codes\" option on the source machine.\n2. Download Code using \"Synchronize Codes\" option on this machine.";
                break;
            case EE_ERR_FILE_NOT_FOUND:
                msg = S + "Element \"" + loc.asText() + "\" file data was not found";
                break;
            case EE_ERR_FILE_INVALID:
                msg = S + "Element \"" + loc.asText() + "\" file data is invalid, you can try the following:\n-Update " ENGINE_NAME " Editor to a newer version\n-Use \"Synchronize Codes\" option to update your codes\n-Use a backup version of your project";
                break;
            case EE_ERR_FILE_READ_ERROR:
                msg = S + "Element \"" + loc.asText() + "\" file reading failed";
                break;
            }
            Gui.msgBox(S, msg);
        }
    }
    return false;
}
/******************************************************************************/
void CodeEditor::save(Source *source, C SourceLoc &loc) {
    if (source) {
        Bool header = source->header, used = source->used();
        if (Source *dest = findSource(loc)) { // find destination source
            if (source != dest) {             // if overwriting existing but different file
                header |= dest->header;
                used |= dest->used();
                if (dest->opened) {
                    Gui.msgBox(S, "Destination file is currently being edited in another source and cannot be overwritten.");
                    return;
                }
                sources.removeData(dest, true); // remove 'dest' source
            }
        }
        Bool different = (source->loc != loc);
        source->save(loc);

        if (CE.options.clang_format_during_save()) {
            if (used || header) {
                validateActiveSources(header); // If the file is used or if it is a header, validate the active sources
                cur()->forcereload();
            }
        } else {
            if ((different && used) || header) {
                validateActiveSources(header); // If saved in a different location and one of the files is used or if it is a header, validate active sources
            }
        }
    }
}
/******************************************************************************/
void CodeEditor::overwrite() {
    if (cur() && !cur()->Const) {
        if (cur()->loc.is())
            save(cur(), cur()->loc);
        else
            cur()->save();
    }
}

void CodeEditor::save() {
    if (cur())
        cur()->save();
}
void CodeEditor::load() {
    if (!load_source.is())
        load_source.create(S, S, S, LoadSource, LoadSource, T).ext(CODE_EXT, "source");
    load_source.load();
}
/******************************************************************************/
} // namespace Edit
} // namespace EE
/******************************************************************************/
