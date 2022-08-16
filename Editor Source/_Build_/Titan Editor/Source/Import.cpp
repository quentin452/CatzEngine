/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
ImporterClass Importer;
/******************************************************************************/

/******************************************************************************/
         ImporterClass::Import::MaterialEx::MaterialEx() : XMaterialEx(&Proj) {}
         void ImporterClass::Import::MaterialEx::check(C Str &path, Str &tex)
         {
            Mems<FileParams> texs=FileParams::Decode(tex);
            FREPA(texs)if(!FExistSystem(texs[i].name))
            {
               Str test=path; test.tailSlash(true)+=GetBaseNoExt(texs[i].name); test+=".webp";
               if(FExistSystem(test))texs[i].name=test;
            }
            tex=FileParams::Encode(texs);
         }
         void ImporterClass::Import::MaterialEx::process(C Str &path)
         {
            if(path.is())
            {
               check(path,    color_map);
               check(path,    alpha_map);
               check(path,     bump_map);
               check(path,     glow_map);
               check(path, emissive_map);
               check(path,   normal_map);
               check(path,   smooth_map);
               check(path,    metal_map);
            }

            if(GetExt(color_map)=="img" || GetExt(normal_map)=="img" || GetExt(smooth_map)=="img" || GetExt(detail_color_map)=="img") // this is 'EE.Material' ("mtrl" format) #MaterialTextureLayout
            {
               Str base0_path=color_map, base1_path=normal_map, base2_path=smooth_map, detail_path=detail_color_map, emissive_path=emissive_map, macro_path;
               base_0      .load(   base0_path); ImageProps(base_0      , &  base_0_id, null, MTRL_BASE_0  );
               base_1      .load(   base1_path); ImageProps(base_1      , &  base_1_id, null, MTRL_BASE_1  );
               base_2      .load(   base2_path); ImageProps(base_2      , &  base_2_id, null, MTRL_BASE_2  );
               detail      .load(  detail_path); ImageProps(detail      , &  detail_id, null, MTRL_DETAIL  );
               macro       .load(   macro_path); ImageProps(macro       , &   macro_id, null, MTRL_MACRO   );
               emissive_img.load(emissive_path); ImageProps(emissive_img, &emissive_id, null, MTRL_EMISSIVE);

               FileParams fp; 
               color_map=base0_path;
               alpha_map.clear();
               emissive_map=emissive_path;

               if(base1_path.is())
               {
                  fp=base1_path; fp.params.New().set("channel", "xy"); normal_map=fp.encode();
               }else
               {
                  normal_map.clear();
               }

               if(base2_path.is())
               {
                  fp=base2_path; fp.params.New().set("channel", "y"); fp.params.New().setName("inverseRGB"); smooth_map=fp.encode();
                  fp=base2_path; fp.params.New().set("channel", "x");                                         metal_map=fp.encode();
                  fp=base2_path; fp.params.New().set("channel", "z");                                          bump_map=fp.encode();
                  fp=base2_path; fp.params.New().set("channel", "w");                                          glow_map=fp.encode();
               }else
               {
                  smooth_map.clear();
                   metal_map.clear();
                    bump_map.clear();
                    glow_map.clear();
               }

               if(detail_path.is()) // #MaterialTextureLayoutDetail
               {
                  fp=detail_path; fp.params.New().set("channel", "w" );                                        detail_color_map =fp.encode();
                  fp=detail_path; fp.params.New().set("channel", "z" ); fp.params.New().setName("inverseRGB"); detail_smooth_map=fp.encode();
                  fp=detail_path; fp.params.New().set("channel", "xy");                                        detail_normal_map=fp.encode();
               }else
               {
                  detail_color_map .clear();
                  detail_smooth_map.clear();
                  detail_normal_map.clear();
               }
               detail_bump_map.clear();
            }else
            {
               Image color, alpha, bump, normal, smooth, metal, glow;
               ImportImage(   color    ,    color_map);
               ImportImage(   alpha    ,    alpha_map);
               ImportImage(    bump    ,     bump_map);
               ImportImage(  normal    ,   normal_map);
               ImportImage(  smooth    ,   smooth_map);
               ImportImage(   metal    ,    metal_map);
               ImportImage(    glow    ,     glow_map);
               ImportImage(emissive_img, emissive_map);

                                    has_textures =CreateBaseTextures(base_0, base_1, base_2, color, alpha, bump, normal, smooth, metal, glow, true, flip_normal_y);
               if(emissive_img.is())has_textures|=TEXF_EMISSIVE;
                                  known_textures =TEXF_BASE|TEXF_EMISSIVE;

               // process textures only if they're added for the first time, otherwise delete them so they won't be saved
               IMAGE_TYPE ct; ImageProps(      base_0, &  base_0_id, &ct, MTRL_BASE_0  ); if(Importer.includeTex(  base_0_id))                                 base_0      .copyTry(base_0      , -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP); else   base_0    .del();
                              ImageProps(      base_1, &  base_1_id, &ct, MTRL_BASE_1  ); if(Importer.includeTex(  base_1_id))                                 base_1      .copyTry(base_1      , -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP); else   base_1    .del();
                              ImageProps(      base_2, &  base_2_id, &ct, MTRL_BASE_2  ); if(Importer.includeTex(  base_2_id))                                 base_2      .copyTry(base_2      , -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP); else   base_2    .del();
                              ImageProps(emissive_img, &emissive_id, &ct, MTRL_EMISSIVE); if(Importer.includeTex(emissive_id)){SetFullAlpha(emissive_img, ct); emissive_img.copyTry(emissive_img, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP);}else emissive_img.del();
            }
         }
      Str ImporterClass::Import::nodeName(int bone_i)C
      {
         if(InRange(bone_i, xskel.bones)) // use 'xskel' if available
         {
          C XSkeleton::Bone &bone=xskel.bones[bone_i];
            return InRange(bone.node, xskel.nodes) ? xskel.nodes[bone.node].name : S;
         }
         if(InRange(bone_i, skel.bones))return skel.bones[bone_i].name; // use 'skel'
         return S;
      }
      Str ImporterClass::Import::nodeUID(int bone_i)C // unique string identifying bone's node !! needs to be the same as 'EditSkeleton.nodeUID' !!
      {
         Str uid;
         if(InRange(bone_i, xskel.bones)) // use 'xskel' if available
         {
            int node_i=xskel.bones[bone_i].node; Memt<int> parents; for(; InRange(node_i, xskel.nodes) && parents.include(node_i); )
            {
             C XSkeleton::Node &node=xskel.nodes[node_i];
               uid+=node.name; // node name
               int parent=node.parent, child_index=0; REP(node_i)if(xskel.nodes[i].parent==parent && xskel.nodes[i].name==node.name)child_index++; if(child_index){uid+=CharAlpha; uid+=child_index;} // node child index in parent (only children with same names are counted)
               uid+='/'; // separator
               node_i=parent;
            }
         }else // use 'skel'
         {
            for(; InRange(bone_i, skel.bones); )
            {
               Str node_name=nodeName(bone_i);
               uid+=node_name; // node name
               int parent=skel.boneParent(bone_i), child_index=0; REP(bone_i)if(skel.boneParent(i)==parent && nodeName(i)==node_name)child_index++; if(child_index){uid+=CharAlpha; uid+=child_index;} // node child index in parent (only children with same names are counted)
               uid+='/'; // separator
               bone_i=parent;
            }
         }
         return uid;
      }
      ::ImporterClass::Import& ImporterClass::Import::set(C UID &elm_id, C UID &parent_id, C Str &file, MODE mode, ELM_TYPE type, C Str &force_name, bool remember_result)
      {
         T.elm_id=elm_id; T.parent_id=parent_id; T.file=file; T.mode=mode; T.type=type; T.force_name=force_name; T.remember_result=remember_result;
         switch(type)
         {
            case ELM_FONT: edit_font.load(Proj.editPath(elm_id)); break;

            case ELM_IMAGE: // check for skybox
            {
               Mems<FileParams> files=FileParams::Decode(T.file);
               if(files.elms()==6)
               {
                  // load helper data from the project
                  File src;
                  UID  image_id;
                  REPA(files)if(DecodeFileName(files[i].name, image_id))if(src.readTry(Proj.editPath(image_id)))
                  {
                     ImageEx &image=images(i);
                     src.copy(image.raw.writeMem());
                     if(Elm *elm=Proj.findElm(image_id))if(ElmImage *data=elm->imageData())image.cube=IsCube(data->mode);
                  }

                  // check if any of the images were not imported
                  if(src.readTry(Proj.editPath(elm_id)))src.copy(raw.writeMem());
               }
            }break;

            case ELM_OBJ: // check if we want to ignore animations
            {
               if(mode==UPDATE // check only for UPDATE because others treat 'elm_id' as target
               && Proj.objToSkel(elm_id).valid())ignore_anims=true; // if object that's being reloaded already had a skeleton set, then ignore loading animations because they were already imported before
            }break;
         }
         return T;
      }
      bool ImporterClass::Import::ApplyVolume(ptr data, int size, flt vol)
      {
         if(data)
         {
            if(size&1)return false; // need 2 byte alignment for a full sample
            short *sample=(short*)data;
            REP(size/SIZE(short))
            {
              *sample=Mid(Round(*sample * vol), SHORT_MIN, SHORT_MAX);
               sample++;
            }
         }
         return true;
      }
      bool ImporterClass::Import::import() // !! this is called on a secondary thread !!
      {
         Mems<FileParams> files=FileParams::Decode(T.file);
         Str file; if(files.elms())file=files[0].name;
         bool all_nodes_as_bones=(type==ELM_ANIM || mode==ANIM); // when importing animations we have to treat all nodes as potential bones, because it's possible that asset developer used helper/dummies to animate nodes which in the base object mesh model got imported as bones due to mesh skinning, however animations don't need the mesh and without the mesh and thus skinning, the nodes would not get detected as bones. Some could get detected as bones if they had animations, but there could be nodes that don't have movement, and are just rotated with 1 keyframe, which would make them different than original bone, but still not detected as a bone), to workaround this we force all nodes as bones, and later we just remove them depending if they're present in the already existing object mesh skeleton from before

         // import
         switch(type)
         {
            case ELM_OBJ:
            {
               if(EE::Import(file, (mode!=ANIM) ? &mesh : null, &skel, (mode!=CLOTH && mode!=ADD && !ignore_anims) ? &anims : null, (mode!=ANIM) ? &mtrls : null, part_mtrl_index, &xskel, all_nodes_as_bones))
               {
                  FixMesh(mesh);
                  mesh.keepOnly(EditMeshFlagAnd); // call after 'FixMesh' which may need/generate some of removed data
                  REPAO(anims).anim.linear(anims[i].fps>=LinearAnimFpsLimit).clip(0, anims[i].anim.length()); // set linear mode and remove any keyframes outside of anim range
                  Str path=GetPath(file); FREPAO(mtrls).process(path);
                  return true;
               }
            }break;

            case ELM_SOUND:
            {
               SOUND_CODEC codec=SOUND_NONE; if(C TextParam *param=files[0].findParam("codec" ))codec =TextSoundCodec(param->value);
               dbl         start=0;          if(C TextParam *param=files[0].findParam("start" ))start =param->asDbl();
               dbl         end   =-1;        if(C TextParam *param=files[0].findParam("end"   ))end   =param->asDbl();
               dbl         length=-1;        if(C TextParam *param=files[0].findParam("length"))length=param->asDbl();
               flt         volume=1;         if(C TextParam *param=files[0].findParam("volume"))volume=param->asFlt(); bool apply_vol=!Equal(volume, 1);
               int         hz=-1;            if(C TextParam *param=files[0].findParam("hz"    ))hz    =param->asInt();
               int         rel_bit_rate=-1 ; C TextParam *rbr=files[0].findParam("relBitRate"); if(!rbr)rbr=files[0].findParam("relativeBitRate"); if(rbr)rel_bit_rate=rbr->asInt();
               if(codec || rel_bit_rate>=0 || start>0 || end>=0 || length>=0 || hz>0 || apply_vol)
               {
                  SoundStream s; if(s.create(file) && s.block())
                  {
                     long  start_sample=((start> 0) ? RoundL(start*s.frequency()) :           0); MIN(start_sample, s.samples());
                     long    end_sample=((end  >=0) ? RoundL(end  *s.frequency()) : s.samples()); MIN(  end_sample, s.samples());
                     long length_sample=Max(0, end_sample-start_sample); if(length>=0)MIN(length_sample, RoundL(length*s.frequency()));
                     if(!codec)codec=s.codec(); // if not specified then use original
                     if(hz<=0)hz=s.frequency();
                     
                     int bit_rate=-1; if(rel_bit_rate>0) // calculate from relative bit rate
                     {
                        bit_rate=long(rel_bit_rate*1000) * s.frequency()/44100 * s.channels()/2;
                     }else
                     if(rel_bit_rate<0 && (Equal(s.codecName(), "opus") || Equal(s.codecName(), "vorbis"))) // get from source (accept only from good lossy compressed codecs, because Raw/Flac will have very big values)
                     {
                        bit_rate=s.bitRate();
                     }
                     if(codec && s.codecName()!=CodecName(codec) // compare names instead of value, to ignore audio container and just check codec, because names always return just the codec
                     || bit_rate>0 && Abs(bit_rate-s.bitRate())>8*1000 // allow 8 kbit tolerance
                     || start_sample>0 // if we want to skip some data
                     || length_sample!=s.samples() // if we want to skip some data
                     || apply_vol // if we want to change volume
                     || hz!=s.frequency() // want to change frequency
                     )
                     {
                        if(codec!=SOUND_WAV
                        && !Equal(CodecName(codec), "opus")
                        && !Equal(CodecName(codec), "vorbis")
                        )codec=SOUND_SND_OPUS; // if selected codec is not supported then use a default one

                        if(bit_rate<=0) // use default bit rate
                        {
                           if(Equal(CodecName(codec), "opus"  ))bit_rate=long(DefaultOpusBitRate  ) * s.frequency()/44100 * s.channels()/2;else
                           if(Equal(CodecName(codec), "vorbis"))bit_rate=long(DefaultVorbisBitRate) * s.frequency()/44100 * s.channels()/2;
                        }
                        int bit_rate_hq=Max(long(DefaultHQBitRate) * s.frequency()/44100 * s.channels()/2, bit_rate);

                        raw.writeMem();
                        byte temp[65536];
                        if(!s.pos(start_sample*s.block()))goto error;
                        long size=length_sample*s.block();
                        if(codec==SOUND_WAV)
                        {
                           SndRawEncoder encoder; if(encoder.create(raw, hz, s.channels(), length_sample))
                           {
                              for(; size>0; )
                              {
                                 int r=s.set(temp, Min(size, (int)SIZE(temp))); if(r<=0)goto error;
                                 if(apply_vol && !ApplyVolume(temp, r, volume))goto error;
                                 if(!encoder.encode(temp, r))goto error; size-=r;
                              }
                              if(!encoder.finish())goto error; return true;
                           }
                        }else
                        if(Equal(CodecName(codec), "opus"))
                        {
                           SndOpusEncoder encoder; if(encoder.create(raw, length_sample, hz, s.channels(), bit_rate_hq)) // prioritize first frame (to avoid pop/clicks for looped audio)
                           {
                              int  frame_size=encoder.block()*encoder.frameSamples();
                              long raw_start=raw.pos();
                              bool first=true;
                              for(; size>0; )
                              {
                                 int r=Min(size, (int)SIZE(temp));
                                 if(first)MIN(r, frame_size); // make sure we encode only the first frame with higher bit rate
                                 r=s.set(temp, r);
                                 if(r<=0)goto error;
                                 if(apply_vol && !ApplyVolume(temp, r, volume))goto error;
                                 if(!encoder.encode(temp, r))goto error; size-=r;
                                 if(first && raw.pos()!=raw_start) // if wrote the first frame
                                 {
                                    first=false;
                                    encoder.bitRate(bit_rate); // set actual desired bit rate
                                 }
                              }
                              encoder.bitRate(bit_rate_hq); // prioritize last frame (to avoid pop/clicks for looped audio)
                              if(!encoder.finish())goto error; return true;
                           }
                        }else
                        if(Equal(CodecName(codec), "vorbis") && s.channels() && s.frequency())
                        {
                           rel_bit_rate=long(bit_rate) * 2/s.channels() * 44100/s.frequency();
                           flt quality=VorbisBitRateToQuality(rel_bit_rate);
                           OggVorbisEncoder encoder; if(encoder.create(raw, hz, s.channels(), quality))
                           {
                              for(; size>0; )
                              {
                                 int r=s.set(temp, Min(size, (int)SIZE(temp))); if(r<=0)goto error;
                                 if(apply_vol && !ApplyVolume(temp, r, volume))goto error;
                                 if(!encoder.encode(temp, r))goto error; size-=r;
                              }
                              if(!encoder.finish())goto error; return true;
                           }
                        }
                     }
                  }
               }
            error:
               File f; if(f.readStdTry(file)){f.copy(raw.writeMem()); return true;}
            }break;

            case ELM_VIDEO:
            {
               File f; if(f.readStdTry(file)){f.copy(raw.writeMem()); return true;}
            }break;

            case ELM_FILE:
            {
               File f; if(f.readStdTry(file))f.copy(raw.writeMem());
            }return true; // allow replacing with empty file

            case ELM_CODE:
            {
               FileText f; if(f.read(file)){f.getAll(code); return true;}
            }break;

            case ELM_ANIM:
            {
               if(GetExt(file)=="anim") // EE animation
               {
                  XAnimation &xanim=anims.New();
                  if(xanim.anim.load(file)){xanim.name=GetBaseNoExt(file); return true;}
               }else // anim inside mesh
               if(EE::Import(file, null, &skel, anims, null, null, &xskel, all_nodes_as_bones)) // skeleton is needed for anims
                  if(anims.elms()>=1) // at least 1 anim
               {
                  if(anims.elms()>1)
                     if(C TextParam *anim_name=files[0].findParam("name"))
                  {
                     REPA(anims)if(anims[i].name==anim_name->value) // if more than 1 then find the one with matching name
                     {
                        Swap(anims[0], anims[i]); // put to first place
                        break;
                     }
                     anims.setNum(1); // remove all other
                  }
                  XAnimation &anim=anims[0];
                  anim.anim.linear(anim.fps>=LinearAnimFpsLimit);
                  if(files.elms())
                  {
                     FileParams &fps=files[0];
                     if(anim.fps>0) // clip !! do this before looping and speed !!
                     {
                      C TextParam *start_frame=fps.findParam("startFrame"),
                                  *  end_frame=fps.findParam(  "endFrame");
                        if(start_frame || end_frame) // clip animation, currently importer will already offset keyframes by 'anim.start', so if we want custom ranges, we need to revert it back
                           anim.anim.clip(start_frame ? start_frame->asFlt()/anim.fps-anim.start : 0,
                                            end_frame ?   end_frame->asFlt()/anim.fps-anim.start : anim.anim.length());
                     }
                     if(C TextParam *p=fps.findParam("loop")) // set looping !! do this after clipping so the clip isn't affected by looping !!
                     {
                        has_loop=true;
                        anim.anim.loop(p->asBool());
                        fps.params.removeData(p); // remove this parameter because we've already applied this change to the animation, so when user modifies manually the looping, and then selectes reload, then looping won't be changed
                     }
                  }
                  anim.anim.clip(0, anim.anim.length()); // remove any keyframes outside of anim range
                  T.file=FileParams::Encode(files); // 'files' could have changed, so adjust the name so the 'elm.srcFile' is set properly
                  return true;
               }
            }break;

            case ELM_IMAGE:
            {
               if(files.elms()==6) // special case of cube map
               {
                  has_color=true ; // assume that they have color
                  has_alpha=false; // ignore alpha for cube maps
                  FREPA(files)
                  {
                     ImageEx &image=images(i); if(!ImportImage(image, files[i].name))
                     {
                        image.raw.pos(0); if(image.ImportTry(image.raw))
                        {
                           if(image.cube)image.crop(image, i*image.w()/6, 0, image.w()/6, image.h()); // crop to i-th face for cubes
                        }
                     }
                     TransformImage(image, files[i].params, true);
                  }

                  // check if any of the images were not imported
                  REPA(images)if(!images[i].is())
                  {
                     Image src; raw.pos(0); if(src.ImportTry(raw)) // try extracting from existing data
                     {
                        bool one=(src.aspect()<Avg(1.0f, 6.0f)); // source is only 1 face, not "6 x face"
                        REPA(images)if(!images[i].is())if(one)src.copyTry(images[i]);else src.crop(images[i], i*src.w()/6, 0, src.w()/6, src.h());
                     }
                     break;
                  }

                  raw.del();

                  // calculate max size
                  int size=0; REPA(images)MAX(size, Max(images[i].w(), images[i].h()));

                  // if actually has some images
                  if(size>0)
                  {
                     Image dest; if(dest.createSoftTry(size*6, size, 1, IMAGE_R8G8B8A8_SRGB)) // create soft RGBA so we can use simple mem copy
                     {
                        // clear to zero in case some images are not found
                        dest.clear();

                        // insert all images into 6*2D image
                        REPA(images)
                        {
                           Image &src=images[i];
                           if(src.is() && src.copyTry(src, size, size, 1, dest.hwType(), IMAGE_SOFT, 1, FILTER_BEST, IC_CLAMP|IC_NO_ALT_TYPE)) // copy to the same size and HW type as dest so simple mem copy can be used
                              if(src.lockRead())
                           {
                              // copy non-compressed 2D face to non-compressed 6*2D
                              int byte_pp=dest.bytePP();
                              REPD(y, size)
                              {
                                 Copy(dest.data() + y*dest.pitch() + i*byte_pp*size,
                                      src .data() + y*src .pitch()                 , byte_pp*size);
                              }
                              src.unlock();
                           }
                        }

                        // compress into WEBP raw data
                        dest.ExportWEBP(raw.writeMem(), 1, 1);
                     }
                  }
                  return true;
               }else
               {
                  Str  ext=GetExt(file);
                  bool transforms=(files.elms()>1 || (files.elms() && files[0].params.elms())); // if want to apply any transforms
                  if(!transforms && (ext=="jpg" || ext=="jpeg" || ext=="webp"/* || ext=="png"*/)) // images are already in accepted format (even though PNG is compressed, we can achieve much better compression with WEBP)
                  {
                     File f; if(!f.readStdTry(file))break; f.copy(raw.writeMem());
                     if(ext=="jpg" || ext=="jpeg"){Image image; raw.pos(0); if(image.ImportJPG (raw)){has_color=HasColor(image); has_alpha=false          ;}}else // JPG never has any alpha
                     if(ext=="png"               ){Image image; raw.pos(0); if(image.ImportPNG (raw)){has_color=HasColor(image); has_alpha=HasAlpha(image);}}else
                     if(ext=="webp"              ){Image image; raw.pos(0); if(image.ImportWEBP(raw)){has_color=HasColor(image); has_alpha=HasAlpha(image);}}
                     return true;
                  }else // import and export as WEBP
                  {
                     Image image; if(LoadImages(null, image, null, T.file)) // proj null because this is called on secondary thread
                     {
                        image.ExportWEBP(raw.writeMem(), 1, 1); has_color=HasColor(image); has_alpha=HasAlpha(image); return true;
                     }
                  }
               }
            }break;

            case ELM_MINI_MAP:
            {
               ImageEx image; if(ImportImage(image, file, IMAGE_R8G8B8A8_SRGB, IMAGE_SOFT, 1))
               {
                  Swap(images.New(), image);
                  return true;
               }
            }break;

            case ELM_MTRL:
            {
               if(GetExt(file)=="mtrl") // EE mtrl
               {
                  Material mtrl; if(mtrl.load(file))
                  {
                     MaterialEx &mtrl_ex=mtrls.New();
                     mtrl_ex.create(mtrl);
                     mtrl_ex.process(S);
                     return true;
                  }
               }else // mtrl inside mesh
               if(EE::Import(file, null, null, null, SCAST(Memc<XMaterial>, mtrls), null))
                  if(mtrls.elms()>=1) // at least 1 mtrl
               {
                  if(mtrls.elms()>1)
                     if(C TextParam *mtrl_name=files[0].findParam("name"))
                  {
                     REPA(mtrls)if(mtrls[i].name==mtrl_name->value) // if more than 1 then find the one with matching name
                     {
                        Swap(mtrls[0], mtrls[i]); // put to first place
                        break;
                     }
                     mtrls.setNum(1); // remove all other
                  }
                  Str path=GetPath(file); FREPAO(mtrls).process(path);
                  return true;
               }
            }break;

            case ELM_FONT:
            {
               return edit_font.make(font);
            }break;
         }
         return false;
      }
   void ImporterClass::ImportDo(Import &import, ImporterClass &importer, int thread_index)
   {
      ThreadMayUseGPUData();
      import.status=import.import(); // after setting this, do not operate on 'import' anymore as it will be removed
      ThreadFinishedUsingGPUData();
   }
      void ImporterClass::ImportElm::set(C UID &elm_id, bool remember_result) {T.elm_id=elm_id; T.remember_result=remember_result;}
  ImporterClass::~ImporterClass() {threads.del();}
   bool ImporterClass::busy()C {return threads.busy();}
   int  ImporterClass::totalLeft()C {return imports.elms()+import_queue.elms();}
   void ImporterClass::ImportFull(ImporterClass &ic) {Proj.drop(ic.import_files, ic.import_target); ic.import_files.clear();}
   void ImporterClass::ImportReplace(ImporterClass &ic) {if(ic.import_files.elms())if(Elm *elm=Proj.findElm(ic.import_target)){elm->setSrcFile(ic.import_files[0]); Proj.elmReload(elm->id); Server.setElmShort(elm->id);}}
   void ImporterClass::ImportAnim(ImporterClass &ic) {ic.importSpecial(ANIM );}
   void ImporterClass::ImportCloth(ImporterClass &ic) {ic.importSpecial(CLOTH);}
   void ImporterClass::ImportAdd(ImporterClass &ic) {ic.importSpecial(ADD  );}
   void ImporterClass::create()
   {
      {
         Node<MenuElm> n;
         n.New().create("Import as child to object"      , ImportFull   , T).desc("This will perform standard importing, and put the element inside selected object.");
         n.New().create("Import as animation to object"  , ImportAnim   , T).desc("This will import animations only and adjust them to selected object.");
         n.New().create("Import as cloth/armor to object", ImportCloth  , T).desc("This will import mesh as cloth/armor to the selected object,\nmaking mesh use the same skeleton as target object.");
         n.New().create("Import and replace object"      , ImportReplace, T).desc("This will replace current object with data from the file.");
         n.New().create("Import and add to object"       , ImportAdd    , T).desc("This will import mesh and add it to current object.");
         Gui+=import_menu.create(n);
      }
      threads.create(true, Max(1, Cpu.threads()-1), 0, "Editor.Import"); // leave 1 thread for the main thread
   }
   void ImporterClass::stop()
   {
      threads     .cancel().wait(); // finish thread processing first before anything else
      imports     .clear();
      import_queue.clear();
      import_files.clear();
   }
   void ImporterClass::importNew(C UID &elm_id, C UID &parent_id, C Str &file, MODE mode, ELM_TYPE type, C Str &force_name, bool remember_result)
   {
      threads.queue(imports.New().set(elm_id, parent_id, file, mode, type, force_name, remember_result), ImportDo, T);
   }
   void ImporterClass::importSpecialFile(C Str &file)
   {
      Str ext=GetExt(file);
      if(ExtType(ext)==EXT_MESH)importNew(import_target, import_target, file, import_mode, ELM_OBJ );else
      if(ext=="anim"           )importNew(import_target, import_target, file, import_mode, ELM_ANIM);
   }
   void ImporterClass::importSpecialDir(C Str &path)
   {
      for(FileFind ff(path); ff(); )switch(ff.type)
      {
         case FSTD_FILE: importSpecialFile(ff.pathName()); break;
         case FSTD_DIR : importSpecialDir (ff.pathName()); break;
      }
   }
   void ImporterClass::importSpecial(MODE mode)
   {
      T.import_mode=mode;
      FREPA(import_files)
      {
         FileInfo fi; if(fi.getSystem(import_files[i]))switch(fi.type)
         {
            case FSTD_FILE: importSpecialFile(import_files[i]); break;
            case FSTD_DIR : importSpecialDir (import_files[i]); break;
         }
      }
   }
   void ImporterClass::import(Elm &target, Memc<Str> &files, C Vec2 &screen_pos)
   {
      if(files.elms())
      {
         import_files.clear(); Swap(import_files, files);
         import_target=target.id;
         import_menu.posRU(screen_pos).activate();
      }
   }
   bool ImporterClass::includeTex(C UID &tex_id)
   {
      if(tex_id.valid())
      {
         SyncLocker locker(lock);
         return texs.binaryInclude(tex_id);
      }
      return false;
   }
   void ImporterClass::excludeTex(C UID &tex_id)
   {
      if(tex_id.valid())
      {
         SyncLocker locker(lock);
         texs.binaryExclude(tex_id);
      }
   }
   void ImporterClass::clearProj()
   {
      stop();
      import_results.clear();
      {SyncLocker locker(lock); texs.clear();}
   }
   void ImporterClass::opened(Project &proj, ElmNode &node)
   {
      if(proj.needUpdate())clearProj();else
      {
         {SyncLocker locker(lock); texs=proj.texs;}
         investigate(node);
      }
   }
   ::ImporterClass::Import* ImporterClass::findImport(C UID &elm_id)
   {
      REPA(imports)
      {
         Import &import=imports[i];
         if(import.mode==UPDATE && import.elm_id==elm_id)return &import;
      }
      return null;
   }
   ::ImporterClass::ImportElm* ImporterClass::findImportQueue(C UID &elm_id)
   {
      FREPA(import_queue)if(import_queue[i].elm_id==elm_id)return &import_queue[i];
      return null;
   }
   bool ImporterClass::inQueue(C UID &elm_id)
   {
      return findImport(elm_id) || findImportQueue(elm_id);
   }
   void ImporterClass::cancelImports(C MemPtr<UID> &sorted_elm_ids)
   {
      REPA(imports) // check all 'imports' in case there are multiple imports for the same element
      {
         Import &import=imports[i]; // remember that this 'import' may be processed on secondary thread
         if(import.mode==UPDATE // only this mode operates on 'elm_id' while others treat it as target
         && sorted_elm_ids.binaryHas(import.elm_id))
         {
            if(threads.cancel(import, ImportDo, T))import.status=0; // if succesfully canceled, then mark as finished, otherwise we need to wait until it will finish on its own
            import.cancel=true;
            if(import.remember_result)*import_results(import.elm_id)=Edit::RELOAD_CANCELED; // if wanted to remember result, then set as canceled
         }
      }
   }
   void ImporterClass::cancel(C MemPtr<UID> &elm_ids)
   {
      if(elm_ids.elms())
      {
         Mems<UID> sorted; sorted=elm_ids; sorted.sort(Compare);
         cancelImports(sorted);
         REPA(import_queue)
         {
            ImportElm &import=import_queue[i];
            if(sorted.binaryHas(import.elm_id))
            {
               if(import.remember_result)*import_results(import.elm_id)=Edit::RELOAD_CANCELED; // if wanted to remember result, then set as canceled
               import_queue.remove(i, true);
            }
         }
      }
   }
   void ImporterClass::getResult(C MemPtr<UID> &elms, MemPtr<Edit::IDParam<Edit::RELOAD_RESULT>> results)
   {
      results.setNum(elms.elms()); // pre-alloc
      FREPA(results)
      {
         Edit::IDParam<Edit::RELOAD_RESULT> &result=results[i];
         result.id=elms[i];
         if(Edit::RELOAD_RESULT *r=import_results.find(result.id))result.value=*r;else
         {
          C Elm *elm=Proj.findElm(result.id);
            if(!elm            )result.value=Edit::RELOAD_ELM_NOT_FOUND;else
            if( elm->importing())result.value=Edit::RELOAD_IN_PROGRESS  ;else
                                result.value=Edit::RELOAD_NOT_REQUESTED;
         }
      }
   }
   void ImporterClass::clearImportResults(C MemPtr<UID> &elm_ids)
   {
      REPA(elm_ids)import_results.removeKey(elm_ids[i]);
   }
   void ImporterClass::forgetResult(C MemPtr<UID> &elm_ids)
   {
      if(elm_ids.elms())
      {
         Mems<UID> sorted; sorted=elm_ids; sorted.sort(Compare);
         REPA(imports)
         {
            Import &import=imports[i]; // remember that this 'import' may be processed on secondary thread
            if(import.mode==UPDATE // only this mode operates on 'elm_id' while others treat it as target
            && import.remember_result // if wanted to remember the result
            && sorted.binaryHas(import.elm_id))import.remember_result=false;
         }
         REPA(import_queue)
         {
            ImportElm &import=import_queue[i];
            if(import.remember_result
            && sorted.binaryHas(import.elm_id))import.remember_result=false;
         }
         clearImportResults(sorted);
      }
   }
   void ImporterClass::reload(C MemPtr<UID> &elm_ids, bool remember_result)
   {
      Mems<UID> sorted; sorted=elm_ids; sorted.sort(Compare);
                         cancelImports     (sorted);
      if(remember_result)clearImportResults(sorted); // since we'll reload elements, we need to clear any previous results, do this after calling 'cancelImports' which may set RELOAD_CANCELED
      REPA(elm_ids) // add in reversed order because elements last in the queue are imported first
      {
       C UID &elm_id=elm_ids[i];
         if(ImportElm *elm=findImportQueue(elm_id))elm->remember_result|=remember_result;
         else                                      import_queue.New().set(elm_id, remember_result);
      }
   }
   void ImporterClass::investigate(Elm &elm)
   {
      if(elm.importing() && !inQueue(elm.id))import_queue.New().set(elm.id);
   }
   void ImporterClass::investigate(ElmNode &node)
   {
      if(!Proj.needUpdate())
      {
         SyncLocker locker(lock);
         REPA(node.children) // go from back
         {
            int  child_i=node.children[i];
            Elm &elm    =Proj.elms[child_i];
            if(ImportRemovedElms || elm.exists())
            {
               investigate(Proj.hierarchy[child_i]);
               investigate(elm);
            }
         }
      }
   }
   void ImporterClass::investigate(Memc<UID> &elm_ids)
   {
      if(!Proj.needUpdate())
      {
         SyncLocker locker(lock);
         FREPA(elm_ids)if(Elm *elm=Proj.findElm(elm_ids[i]))if(ImportRemovedElms || elm->finalExists())
         {
            int index=Proj.elms.validIndex(elm); if(InRange(index, Proj.hierarchy))
            {
               investigate(Proj.elms     [index]);
               investigate(Proj.hierarchy[index]);
            }
         }
      }
   }
   void ImporterClass::processUpdate(Import &import)
   {
      if(Elm *elm=Proj.findElm(import.elm_id))
      {
         Proj.setListCurSel();
         Proj.closeElm(import.elm_id);
         elm->importing(false);
         TimeStamp time; time.getUTC();
         switch(elm->type)
         {
            case ELM_MTRL: if(import.mtrls.elms())
            {
               FileParams fp=import.file; fp.getParam("name").value=import.mtrls[0].name;
               Proj.setMtrl(*elm, import.mtrls[0], fp.encode());
               Server.setElmLong(elm->id);
            }break;

            case ELM_IMAGE:
            {
               elm->setSrcFile(import.file, time);
               import.raw.pos(0); Proj.imageSet(elm->id, import.raw, import.has_color, import.has_alpha);
            }break;

            case ELM_SOUND:
            case ELM_VIDEO:
            case ELM_FILE :
            {
               elm->setSrcFile(import.file, time);
               import.raw.pos(0); Proj.fileSet(elm->id, import.raw);
            }break;

            case ELM_CODE:
            {
               elm->setSrcFile(import.file, time);
               Proj.codeSet(elm->id, import.code);
            }break;

            case ELM_FONT:
            {
               Proj.elmChanging(*elm);
               elm->fontData()->newVer();
               Save(import.font, Proj.gamePath(*elm)); Proj.savedGame(*elm);
               Proj.elmChanged(*elm);
               Server.setElmLong(elm->id);
            }break;

            case ELM_ANIM:
            {
               if(import.anims.elms())
               if(ElmAnim *anim_data=elm->animData())
               {
                  Mems<FileParams>     file_params=FileParams::Decode(import.file);
                  Mems<FileParams> old_file_params=FileParams::Decode(anim_data->imported_file_params);
                  FileParams *fps=file_params.data(), *old_fps=old_file_params.data();
                  XAnimation &xanim=import.anims[0];
                   Animation & anim=xanim.anim;
                  anim_data->newVer();
                  anim_data->fps=xanim.fps;
                  anim_data->file_time=time; // file was changed
                  anim_data->setSrcFile(import.file, time);
                  anim_data->setImportedFileParams();
                  if(import.has_loop) // if import has information about looping then use it
                  {
                     anim_data->loop(anim.loop()); anim_data->loop_time=time; // set from imported animation
                  }else anim.loop(anim_data->loop()); // otherwise keep the old setting
                  anim.linear(anim_data->linear()); // keep old linear

                  // set events !! AFTER CLIP BUT BEFORE 'speedTime', 'offsetTime', 'speed'
                  if(!anim.events.elms()) // if new animation doesn't have events, then check old anim for them
                  {
                     Animation old;
                     {
                        Animation temp; if(Animation *anim=Proj.getAnim(elm->id, temp))old.loop(anim->loop()).linear(anim->linear()).length(anim->length(), false).events=anim->events; // we just need events
                     }
                     if(old.events.elms()) // if old had events
                     {
                        // convert from old to absolute, have to process in reversed order
                        if(old_fps)
                        {
                           if(C TextParam *p=old_fps->findParam("speed"))
                              if(flt speed=p->asFlt())
                                 old.length(old.length()*speed, true);

                                              if(C TextParam *p=old_fps->findParam("offsetTime" ))old.offsetTime(p->asFlt());
                           if(anim_data->fps>0)if(C TextParam *p=old_fps->findParam("offsetFrame"))old.offsetTime(p->asFlt()/anim_data->fps);

                           REPA(old_fps->params) // process in reversed order
                           {
                            C TextParam &p=old_fps->params[i];
                              if(p.name=="speedTime")
                              {
                                 Vec v=p.asVec(); flt speed=v.x, start=v.y, end=v.z;
                                 if(speed)old.scaleTime(start, end ? start+(end-start)/speed : old.length(), speed);
                              }else
                              if(p.name=="speedFrame")
                              {
                                 if(anim_data->fps>0)
                                 {
                                    Vec v=p.asVec(); flt speed=v.x, start=v.y/anim_data->fps, end=v.z/anim_data->fps;
                                    if(speed)old.scaleTime(start, end ? start+(end-start)/speed : old.length(), speed);
                                 }
                              }
                           }
                        }

                        if(anim_data->fps>0)
                        {
                           flt offset=0;
                           if(old_fps)if(C TextParam *start_frame=old_fps->findParam("startFrame"))offset+=start_frame->asFlt()/anim_data->fps-xanim.start; // currently importer will already offset keyframes by 'anim.start', so if we want custom ranges, we need to revert it back
                           if(    fps)if(C TextParam *start_frame=    fps->findParam("startFrame"))offset-=start_frame->asFlt()/anim_data->fps-xanim.start; // currently importer will already offset keyframes by 'anim.start', so if we want custom ranges, we need to revert it back
                           if(offset )REPAO(old.events).time+=offset;
                        }

                        Swap(anim.events, old.events);
                        REPA(anim.events)Clamp(anim.events[i].time, 0, anim.length());
                     }
                  }

                  if(fps) // 'speedTime' !! BEFORE ROOT !!
                  {
                     FREPA(fps->params) // process in order
                     {
                      C TextParam &p=fps->params[i];
                        if(p.name=="speedTime") // adjust speed for specified time range !! do this after clipping so the clip isn't affected by speed !!
                        {
                           Vec v=p.asVec(); flt speed=v.x, start=v.y, end=v.z;
                           if(speed)anim.scaleTime(start, end ? end : anim.length(), 1/speed); // if no end specified then use until end of anim
                        }else
                        if(p.name=="speedFrame")
                        {
                           if(anim_data->fps>0)
                           {
                              Vec v=p.asVec(); flt speed=v.x, start=v.y/anim_data->fps, end=v.z/anim_data->fps;
                              if(speed)anim.scaleTime(start, end ? end : anim.length(), 1/speed); // if no end specified then use until end of anim
                           }
                        }
                     }
                  }

                  if(Elm *skel_elm=Proj.findElm(anim_data->skel_id))if(ElmSkel *skel_data=skel_elm->skelData()) // load target skeleton
                  {
                     anim_data->transform=skel_data->transform; // set desired transform to match target skeleton

                       C Skeleton           *skel=Skeletons(Proj.gamePath(skel_elm->id));
                     EditSkeleton edit_skel; edit_skel.load(Proj.editPath(skel_elm->id));
                     Matrix m=skel_data->transform();

                     if(import.skel.is())
                     {
                        // transform first, in case some methods rely on correct scale
                        import.skel.transform(m);
                               anim.transform(m, import.skel, false);

                     /* First we have to make skel/anim compatible with 'EditSkeleton', we do this by removing any extra bones from import to match 'EditSkeleton'
                        Animation may contain extra bones not present in the Object's EditSkeleton so just remove them
                        This is important, because if animation skeleton has "A->B->C" bones, but target has "A->C" bones, then in animations "C" bones are stored relative to "B" parent,
                           but they need to be stored relative to "A"
                        This is only for few special cases where Animation FBX files have more bones detected than the base Mesh+Skel FBX */
                        Map<Str8, int> skel_to_node(CompareCI); // map that converts SkelBone name -> EditSkeleton node index
                        bool remove=false; // if we've found any bone that isn't present in EditSkeleton and needs to be removed
                        // remove bones which nodes are not found in EditSkeleton
                        REPA(import.skel.bones)
                        {
                           int node=edit_skel.findNodeI(import.nodeName(i), import.nodeUID(i));
                           if( node>=0)*skel_to_node(import.skel.bones[i].name)=node;else remove=true; // set only those that were found, others will be removed below
                        }
                        if(remove) // if we need to remove some bones
                        {
                           Skeleton temp=import.skel; REPA(temp.bones)if(!skel_to_node.find(temp.bones[i].name))temp.removeBone(i); // if this bone is not present in 'EditSkeleton' then remove it
                           anim.adjustForSameTransformWithDifferentSkeleton(import.skel, temp); // this will also remove 'anim.bones' not found in skeleton
                           Swap(temp.bones, import.skel.bones);
                        }
                        // convert to EditSkeleton from nodes (with bone names set as node index)
                        {
                           // rename from skel bone names to name set from node index to match what we will set in 'temp' skeleton, for example name "leg" gets renamed to "1" if the node index==1
                           REPA(import.skel.bones){SkelBone &bone=import.skel.bones[i]; Set(bone.name, TextInt(*skel_to_node(bone.name)));}
                           REPA(       anim.bones){AnimBone &bone=       anim.bones[i]; Set(bone.name, TextInt(*skel_to_node(bone.name)));}
                           Skeleton temp; edit_skel.set(temp); temp.transform(m);

                           // adjust skeleton pose
                           // TODO: what about if edit_skel/temp have more bones than import.skel? Or if they have different parents
                           anim.adjustForSameSkeletonWithDifferentPose(import.skel, temp);
                           Swap(temp.bones, import.skel.bones);
                        }
                        if(fps)
                        {
                           // transform / rotate !! BEFORE ROOT !!
                         C Skeleton &sk=import.skel;
                           FREPA(fps->params)
                           {
                            C TextParam &p=fps->params[i];
                              if(p.name=="rotX")anim.transform(Matrix().setRotateX(DegToRad(p.asFlt())), sk, true);else
                              if(p.name=="rotY")anim.transform(Matrix().setRotateY(DegToRad(p.asFlt())), sk, true);else
                              if(p.name=="rotZ")anim.transform(Matrix().setRotateZ(DegToRad(p.asFlt())), sk, true);
                           }
                        }
                        {
                           Mems<Mems<IndexWeight>> weights; edit_skel.set(weights, import.skel, *skel, EditSkeleton::BONE_NAME_IS_NODE_INDEX);
                           int root=edit_skel.root;
                           if(anim_data->flag&ElmAnim::ROOT_FROM_BODY) // if want from body, then override 'root' bone index, and set custom
                           {
                              int bone=Max(0, skel->findBoneI(BONE_SPINE));
                              if(InRange(bone, skel->bones))root=edit_skel.boneToNode(skel->bones[bone].name);
                           }
                           anim.adjustForSameTransformWithDifferentSkeleton(import.skel, *skel, root, weights, anim_data->rootFlags()|(edit_skel.rootZero() ? 0 : ROOT_BONE_POSITION|ROOT_START_IDENTITY)); // don't set ROOT_BONE_POSITION for 'rootZero' because we actually want to force zero bone position, this is needed in case original skeleton root bone pos is zero, and skeleton is transformed with custom position offset, in which case 'import.skel' root bone pos will not be zero and 'edit_skel' (in original transform) root bone pos is zero
                           anim_data->setRoot(anim);
                        }
                     }else
                     {
                        anim.setBoneNameTypeIndexesFromSkeleton(*skel);
                        anim.transform(m, *skel, false);
                     }

                     if(fps)
                     {  // 'offsetTime' !! AFTER ROOT !!
                                           if(C TextParam *p=fps->findParam("offsetTime" ))anim.offsetTime(-p->asFlt());
                        if(anim_data->fps>0)if(C TextParam *p=fps->findParam("offsetFrame"))anim.offsetTime(-p->asFlt()/anim_data->fps);
                     }
                     // optimize (after transform because scale affects position key removal, after 'adjustForSameTransformWithDifferentSkeleton' so it can operate on highest precision and to cleanup keyframes generated by it, after 'offsetTime' to cleanup keyframes generated by it)
                     {
                        flt angle_eps=EPS_ANIM_ANGLE, pos_eps=EPS_ANIM_POS, scale_eps=EPS_ANIM_SCALE;
                        if(fps)if(C TextParam *optimize=fps->findParam("optimize")){flt o=optimize->asFlt(); angle_eps*=o; pos_eps*=o; scale_eps*=o;}
                        anim.optimize(angle_eps, pos_eps, scale_eps);
                     }
                     if(fps)
                     {
                        // 'speed' !! AFTER clipping, 'speedTime', 'offsetTime' so they're not affected by speed !!
                        if(C TextParam *p=fps->findParam("speed"))
                           if(flt speed=p->asFlt())
                              anim.length(anim.length()/speed, true);

                        // mirror
                        if(C TextParam *mirror=fps->findParam("mirror"))if(mirror->asBool1())anim.mirror(*skel);

                        // delete keys at the end !! after clipping and root !!
                        if(C TextParam *p=fps->findParam("delEndKeys"))DelEndKeys(anim);
                     }
                  }

                  // save and send
                  Str path=Proj.gamePath(elm->id);
                  Proj.elmChanging(*elm);
                  Save(anim, path); Proj.savedGame(*elm, path);
                  Proj.elmChanged(*elm);
                  Server.setElmLong(elm->id);
               }
            }break;

            case ELM_OBJ: if(ElmObj *obj_data=elm->objData())
            {
               obj_data->newVer();
               obj_data->setSrcFile(import.file, time);
               Skeleton &new_skel=import.skel;

               if(import.mesh.is())Proj.getObjMeshElm(elm->id, false, false); // need to have mesh     element to insert mesh
               if(   new_skel.is())Proj.getObjSkelElm(elm->id, false, false); // need to have skeleton element to insert skeleton

               // update
               if(Elm *mesh_elm=Proj.findElm(obj_data->mesh_id))
                  if(ElmMesh *mesh_data=mesh_elm->meshData())
               {
                  FileParams fp=import.file; // !! watch out because this has param 'name' changed throughout the codes !!

                              Proj.elmChanging(*     elm);
                  if(mesh_elm)Proj.elmChanging(*mesh_elm);

                  // update skeleton
                  Elm *skel_elm=null;
                  if(new_skel.is()) // only if there's a new skeleton, otherwise keep the current
                     if(skel_elm=Proj.findElm(mesh_data->skel_id))
                        if(ElmSkel *skel_data=skel_elm->skelData())
                  {
                     if(skel_elm)Proj.elmChanging(*skel_elm);

                     skel_data->newVer();
                     skel_data->setSrcFile(import.file, time);
                     skel_data->file_time=time;

                     Matrix m=skel_data->transform();

                     Str          skel_game_path=Proj.gamePath(skel_elm->id), skel_edit_path=Proj.editPath(skel_elm->id);
                     Skeleton    *old_skel=Skeletons(skel_game_path);
                     EditSkeleton old_edit_skel, new_edit_skel; old_edit_skel.load(skel_edit_path); new_edit_skel.create(new_skel, &import.xskel); // !! create before transform is applied !!
                     old_edit_skel.sortBoneWeights(); // needed because we want to search bone weights by importance
                     new_skel.transform(m); // !! transform after creating 'EditSkeleton' !!

                     // keep Skeleton params and set bone weights from new to old
                     Mems<Mems<IndexWeight>> bone_weights; bone_weights.setNum(new_skel.bones.elms());
                     {
                        // slots
                        new_skel.slots=old_skel->slots; REPA(new_skel.slots)
                        {
                           SkelSlot &slot=new_skel.slots[i];

                           for(int old_bone_i=slot.bone; C SkelBone *old_bone=old_skel->bones.addr(old_bone_i); old_bone_i=old_skel->boneParent(old_bone_i)) // iterate bone and its parents
                              if(C EditSkeleton::Bone *old_edit_bone=old_edit_skel.findBone(old_bone->name))
                                 FREPA(*old_edit_bone) // iterate all Bone->Node links, starting from most important (requires 'sortBoneWeights' above)
                           {
                                                 int  old_edit_node_i=(*old_edit_bone)[i].index;
                              if(C EditSkeleton::Node *old_edit_node  =  old_edit_skel.nodes.addr(old_edit_node_i))
                              {
                                 int new_bone =new_edit_skel.nodeToBone(new_edit_skel.findNodeI(old_edit_node->name, old_edit_skel.nodeUID(old_edit_node_i)));
                                 if( new_bone>=0){slot.bone=new_bone; goto found;}
                              }
                           }
                           slot.bone=0xFF; // not found
                        found:;

                           for(int old_bone_i=slot.bone1; C SkelBone *old_bone=old_skel->bones.addr(old_bone_i); old_bone_i=old_skel->boneParent(old_bone_i)) // iterate bone and its parents
                              if(C EditSkeleton::Bone *old_edit_bone=old_edit_skel.findBone(old_bone->name))
                                 FREPA(*old_edit_bone) // iterate all Bone->Node links, starting from most important (requires 'sortBoneWeights' above)
                           {
                                                 int  old_edit_node_i=(*old_edit_bone)[i].index;
                              if(C EditSkeleton::Node *old_edit_node  =  old_edit_skel.nodes.addr(old_edit_node_i))
                              {
                                 int new_bone =new_edit_skel.nodeToBone(new_edit_skel.findNodeI(old_edit_node->name, old_edit_skel.nodeUID(old_edit_node_i)));
                                 if( new_bone>=0){slot.bone1=new_bone; goto found1;}
                              }
                           }
                           slot.bone1=0xFF; // not found
                        found1:;
                        }
                        // bones
                        REPAD(new_skel_bone_i, new_skel.bones)
                        {
                           int new_node_i=new_edit_skel.boneToNode(new_skel_bone_i); if(new_node_i>=0)
                           {
                              int old_node_i=old_edit_skel.findNodeI(new_edit_skel.nodes[new_node_i].name, new_edit_skel.nodeUID(new_node_i)); if(old_node_i>=0)
                              {
                                 int best_old_skel_bone_i=-1; flt weight; REPAD(b, old_edit_skel.bones)
                                 {
                                  C EditSkeleton::Bone &old_edit_skel_bone=old_edit_skel.bones[b]; REPAD(bw, old_edit_skel_bone)
                                    {
                                     C IndexWeight &iw=old_edit_skel_bone[bw]; if(iw.index==old_node_i)
                                       {
                                          int old_skel_bone_i=old_skel->findBoneI(old_edit_skel_bone.name); if(old_skel_bone_i>=0)
                                          {
                                             bone_weights[new_skel_bone_i].New().set(old_skel_bone_i, iw.weight);
                                             if(best_old_skel_bone_i<0 || iw.weight>weight){best_old_skel_bone_i=old_skel_bone_i; weight=iw.weight;}
                                          }
                                       }
                                    }
                                 }
                                 if(best_old_skel_bone_i>=0) // copy 'SkelBone' params from an old bone with highest weight
                                 {
                                  C SkelBone & src_bone=old_skel->bones[best_old_skel_bone_i];
                                    SkelBone &dest_bone=new_skel.bones[     new_skel_bone_i];
                                    dest_bone.width=src_bone.width;
                                  //dest_bone.frac =src_bone.frac ;
                                    dest_bone.flag =src_bone.flag ;
                                 }
                              }  
                           }
                        }
                     }
                     Proj.adjustAnimations(skel_elm->id, old_edit_skel, *old_skel, new_skel, bone_weights); // !! convert animations before saving skeleton which modifies 'old_skel' which is a pointer to cache element !!

                     Save(new_edit_skel, skel_edit_path);
                     Save(new_skel     , skel_game_path); Proj.savedGame(*skel_elm, skel_game_path);
                     Server.setElmFull(mesh_data->skel_id);

                     // process animations (they will be available only during first import, and not for "reload", so we don't need to do any fancy conversions)
                     FREPA(import.anims)
                     {
                        XAnimation &xanim=import.anims[i];
                        Elm        & anim=Proj.Project::newElm(xanim.name.is() ? xanim.name : "Animation", elm->id, ELM_ANIM);
                        if(ElmAnim * anim_data=anim.animData())
                        {
                           fp.getParam("name").value=xanim.name;
                           anim_data->newData();
                           anim_data->skel_id  =skel_elm ->id;
                           anim_data->transform=skel_data->transform;
                           anim_data->loop     (xanim.anim.loop()).linear(xanim.anim.linear());
                           anim_data->src_file =fp.encode(); anim_data->setImportedFileParams();
                           anim_data->fps      =xanim.fps;
                        }
                        xanim.anim.transform(m, new_skel, false);
                        xanim.anim.optimize(); // optimize after transform
                        Save(xanim.anim, Proj.gamePath(anim)); Proj.savedGame(anim);
                        Server.setElmFull(anim.id);
                     }
                  }

                  // setup materials before mesh
                  Memt<UID> mtrls;
                  FREPA(import.mtrls)
                  {
                     Elm *found_mtrl=null; int match=0;
                     Import::MaterialEx &src=import.mtrls[i];
                     fp.getParam("name").value=src.name; Str mtrl_src_file=fp.encode();
                     REPA(Proj.elms) // find that material
                     {
                        Elm &mtrl=Proj.elms[i];
                        if(ElmMaterial *mtrl_data=mtrl.mtrlData())if(mtrl.finalExists())
                        {
                           int m=0;
                           if(mtrl.parent_id==elm->id) // material is inside this object
                           {
                              if(EqualPath(mtrl_data->src_file,          mtrl_src_file))m=3;else // full path the same - perfect match
                              if(  GetBase(mtrl_data->src_file)==GetBase(mtrl_src_file))m=2;     // base name the same
                           }else
                           {
                              if(EqualPath(mtrl_data->src_file,          mtrl_src_file))m=1; // full path the same, but not inside the object, perhaps this material is used for something else, prefer materials inside the object
                           }
                           if(m>match){match=m; found_mtrl=&mtrl;}
                        }
                     }
                     if(!found_mtrl) // if haven't found then create a new one
                     {
                        found_mtrl=&Proj.newMtrl(src, elm->id, mtrl_src_file);
                        Server.setElmFull(found_mtrl->id);
                     }
                     mtrls.add(found_mtrl->id);
                  }

                  // update mesh
                  mesh_data->newVer();
                  mesh_data->setSrcFile(import.file, time);
                  mesh_data->file_time=time;
                  int pmi=0; FREPD(l, import.mesh.lods()) // set materials
                  {
                     MeshLod &lod=import.mesh.lod(l); FREPA(lod)
                     {
                        if(InRange(pmi, import.part_mtrl_index))
                        {
                           int mtrl_index=import.part_mtrl_index[pmi];
                           if(InRange(mtrl_index, mtrls))lod.parts[i].material(Proj.gamePath(mtrls[mtrl_index]));
                        }
                        pmi++;
                     }
                  }
                  Str  edit_path=Proj.editPath(mesh_elm->id);
                  Mesh old_mesh; if(Load(old_mesh, edit_path, Proj.game_path))KeepParams(old_mesh, import.mesh);
                  Save(import.mesh, edit_path, Proj.game_path); // save

                  // game mesh
                  Skeleton *body_skel; Proj.getMeshSkels(mesh_data, null, &body_skel);
                  Mesh game_mesh; EditToGameMesh(import.mesh, game_mesh, body_skel, Proj.getEnum(mesh_data->draw_group_id), &NoTemp(mesh_data->transform()));
                  Save(game_mesh, Proj.gamePath(mesh_elm->id)); Proj.savedGame(*mesh_elm); // save
                  mesh_data->from(game_mesh);
                  Server.setElmFull(obj_data->mesh_id);

                  // notify about change
                              Proj.elmChanged(*     elm);
                  if(mesh_elm)Proj.elmChanged(*mesh_elm);
                  if(skel_elm)Proj.elmChanged(*skel_elm);

                  // send to server
                  Server.setElmShort(elm->id);
               }
            }break;

            case ELM_MINI_MAP:
            {
               if(import.images.elms()==1)
                  if(ElmMiniMap *mini_map_data=elm->miniMapData())
                     if(MiniMapVer *ver=Proj.miniMapVerGet(elm->id))
                        if(ver->images.elms())
                           if(int image_size=ver->settings.image_size)
               {
                  Proj.elmChanging(*elm);
                  mini_map_data->newVer();
                  mini_map_data->setSrcFile(import.file, time);
                  RectI images=ver->images.last(); REPA(ver->images)images|=ver->images[i];
                  VecI2 images_size=image_size*(images.size()+1);
                  Image &all=import.images[0], single, temp;
                  if(all.copyTry(all, images_size.x, images_size.y, 1, IMAGE_R8G8B8A8_SRGB, IMAGE_SOFT, 1)
                  && single.createSoftTry(image_size, image_size, 1, IMAGE_R8G8B8A8_SRGB))
                  {
                     ver->changed=true;
                     ver->time=time; // update time of mini map
                     Server.setMiniMapSettings(elm->id, ver->settings, ver->time); // send updated mini map settings first, as images will be received only if their timestamp matches the settings
                     REPA(ver->images)
                     {
                      C VecI2 &image_pos=ver->images[i];
                        int ox=(image_pos.x-images.min.x            )*image_size,
                            oy=(            images.max.y-image_pos.y)*image_size;
                        REPD(y, single.h())
                        REPD(x, single.w())single.pixel(x, y, all.pixel(x+ox, y+oy));
                        if(single.copyTry(temp, -1, -1, -1, IMAGE_BC1_SRGB, IMAGE_2D, 1))
                        {
                           temp.save(Proj.gamePath(elm->id).tailSlash(true)+image_pos);
                           Synchronizer.setMiniMapImage(elm->id, image_pos);
                        }
                     }
                     Server.setElmShort(elm->id);
                     Proj.elmChanged(*elm);
                  }
               }
            }break;
         }
         Proj.setList(); // refresh list because 'importing' affects elm color, and ELM_OBJ could have created new elements
      }
   }
   void ImporterClass::processCloth(Import &import)
   {
      if(import.mesh.is())
      if(Elm     *obj_elm=Proj.findElm(import.elm_id, ELM_OBJ))
      if(ElmObj  *obj_data=obj_elm->objData())
      if(Elm     *body_elm=Proj.findElm(obj_data->mesh_id))
      if(ElmMesh *body_mesh_data=body_elm->meshData())
      {
         if(Elm *parent=Proj.findElm(import.parent_id))parent->opened(true);
         Elm &cloth=Proj.Project::newElm(GetBaseNoExt(import.file), import.parent_id, ELM_OBJ);
         if(Elm *cloth_mesh=Proj.getObjMeshElm(cloth.id))
            if(ElmMesh *mesh_data=cloth_mesh->meshData())
         {
            mesh_data->newData();
            mesh_data->body_id  =      obj_data->mesh_id  ;
            mesh_data->transform=body_mesh_data->transform;
         }
         import.elm_id=cloth.id; // change id to cloth because we want to set it
         processUpdate(import); // this will handle setting elm list and sending to server
      }
   }
   void ImporterClass::processAnim(Import &import)
   {
    //if(import.skel.is()) don't check this because we can import also EE.Animation ('anim' file format) which doesn't contain a skeleton
      if(import.anims.elms())
      if(  Elm      *obj_elm=Proj.findElm(import.elm_id, ELM_OBJ))
      if(  ElmObj   *obj_data=obj_elm->objData())
      if(  Elm      *mesh_elm=Proj.findElm(obj_data->mesh_id))
      if(  ElmMesh  *mesh_data=mesh_elm->meshData())
      if(  Elm      *skel_elm=Proj.findElm(mesh_data->skel_id))
      if(  ElmSkel  *skel_data=skel_elm->skelData())
      if(C Skeleton *skel=Skeletons(Proj.gamePath(skel_elm->id)))
      {
         FileParams fp=import.file;
         EditSkeleton edit_skel; edit_skel.load(Proj.editPath(skel_elm->id)); // 'edit_skel' is always in identity matrix
         Matrix m=skel_data->transform();

         if(import.skel.is())
         {
            // transform first, in case some methods rely on correct scale
                         import.skel.transform(m);
            REPAO(import.anims).anim.transform(m, import.skel, false);

            Map<Str8, int> skel_to_node(CompareCI); // map that converts SkelBone name -> EditSkeleton node index
            bool remove=false; // if we've found any bone that isn't present in EditSkeleton and needs to be removed
            // remove bones which nodes are not found in EditSkeleton
            REPA(import.skel.bones)
            {
               int node=edit_skel.findNodeI(import.nodeName(i), import.nodeUID(i));
               if( node>=0)*skel_to_node(import.skel.bones[i].name)=node;else remove=true; // set only those that were found, others will be removed below
            }
            if(remove) // if we need to remove some bones
            {
               Skeleton temp=import.skel; REPA(temp.bones)if(!skel_to_node.find(temp.bones[i].name))temp.removeBone(i); // if this bone is not present in 'EditSkeleton' then remove it
               REPAO(import.anims).anim.adjustForSameTransformWithDifferentSkeleton(import.skel, temp); // this will also remove 'anim.bones' not found in skeleton
               Swap(temp.bones, import.skel.bones);
            }
            // convert to EditSkeleton from nodes (with bone names set as node index)
            {
               // rename from skel bone names to name set from node index to match what we will set in 'temp' skeleton, for example name "leg" gets renamed to "1" if the node index==1
                                                                        REPA(import.skel.bones){SkelBone &bone=import.skel.bones[i]; Set(bone.name, TextInt(*skel_to_node(bone.name)));}
               REPA(import.anims){Animation &anim=import.anims[i].anim; REPA(       anim.bones){AnimBone &bone=       anim.bones[i]; Set(bone.name, TextInt(*skel_to_node(bone.name)));}}
               Skeleton temp; edit_skel.set(temp); temp.transform(m);

               // adjust skeleton pose
               // TODO: what about if edit_skel/temp have more bones than import.skel? Or if they have different parents
               REPAO(import.anims).anim.adjustForSameSkeletonWithDifferentPose(import.skel, temp);
               Swap(temp.bones, import.skel.bones);
            }
            {
               Mems<Mems<IndexWeight>> weights; edit_skel.set(weights, import.skel, *skel, EditSkeleton::BONE_NAME_IS_NODE_INDEX);
               REPAO(import.anims).anim.adjustForSameTransformWithDifferentSkeleton(import.skel, *skel, edit_skel.root, weights, (edit_skel.rootZero() ? 0 : ROOT_BONE_POSITION|ROOT_START_IDENTITY));
            }
         }else
         {
            REPA(import.anims)
            {
               Animation &anim=import.anims[i].anim;
               anim.setBoneNameTypeIndexesFromSkeleton(*skel);
               anim.transform(m, *skel, false);
            }
         }

         // optimize (after transform because scale affects position key removal, after 'adjustForSameTransformWithDifferentSkeleton' so it can operate on highest precision and to cleanup keyframes generated by it)
         {
            REPAO(import.anims).anim.optimize();
         }

         Proj.setListCurSel();
         FREPA(import.anims)
         {
            XAnimation &xanim=import.anims[i];

            // adjust name
            if(!import.force_name.is() && import.anims.elms()==1 && (xanim.name=="Take 01" || xanim.name=="Take 001" || xanim.name=="Default Take" || xanim.name=="C4D Animation Take" || xanim.name=="mixamo.com" || xanim.name=="Unreal Take" || xanim.name=="Scene" || xanim.name=="Base Stack"))
            {
               import.force_name=Replace(GetBaseNoExt(import.file), '_', ' ');
               REPA(import.force_name)if(import.force_name[i]=='@') // check for unity style file names (one anim per file, name "take 01" or "take 001"), file name "Forest Wolf@Bite Attack.FBX"
               {
                  import.force_name.remove(0, i+1);
                  break;
               }
            }

            // create element
            Elm        &anim=Proj.Project::newElm(import.force_name.is() ? import.force_name : xanim.name.is() ? xanim.name : GetBaseNoExt(FileParams(import.file).name), import.parent_id, ELM_ANIM);
            if(ElmAnim *anim_data=anim.animData())
            {
               fp.getParam("name").value=xanim.name;
               anim_data->newData();
               anim_data->skel_id  =skel_elm->id;
               anim_data->transform=skel_data->transform;
               anim_data->loop     (xanim.anim.loop()).linear(xanim.anim.linear());
               anim_data->src_file =fp.encode(); anim_data->setImportedFileParams();
               anim_data->fps      =xanim.fps;
            }

            // save and send
            Save(xanim.anim, Proj.gamePath(anim.id)); Proj.savedGame(anim);
            Server.setElmFull(anim.id);
         }
         Proj.setList();
      }
   }
   void ImporterClass::processAdd(Import &import)
   {
      Proj.closeElm(import.elm_id);
      TimeStamp time; time.getUTC();
      if(Elm *elm=Proj.findElm(import.elm_id))switch(elm->type)
      {
         case ELM_OBJ:
         {
            if(ElmObj *obj_data=elm->objData())
            {
               Proj.setListCurSel();
               obj_data->newVer();
               obj_data->setSrcFile(FileParams::Merge(obj_data->src_file, import.file), time); // add to file list

               if(import.mesh.is())Proj.getObjMeshElm(elm->id, false, false); // need to have mesh     element to insert mesh
             //if(import.skel.is())Proj.getObjSkelElm(elm.id, false, false); // need to have skeleton element to insert skeleton

               // update
               if(Elm *mesh_elm=Proj.findElm(obj_data->mesh_id))
                  if(ElmMesh *mesh_data=mesh_elm->meshData())
               {
                  // update skeleton
                //Elm *skel_elm=null; // currently this is not done

                              Proj.elmChanging(*     elm);
                  if(mesh_elm)Proj.elmChanging(*mesh_elm);
                //if(skel_elm)Proj.elmChanging(*skel_elm);

                  // first setup materials
                  FileParams fp=import.file;
                  Memt<UID> mtrls;
                  FREPA(import.mtrls)
                  {
                     Elm *found_mtrl=null; int match=0;
                     Import::MaterialEx &src=import.mtrls[i];
                     fp.getParam("name").value=src.name; Str mtrl_src_file=fp.encode();
                     REPA(Proj.elms) // find that material
                     {
                        Elm &mtrl=Proj.elms[i];
                        if(ElmMaterial *mtrl_data=mtrl.mtrlData())if(mtrl.finalExists())
                        {
                           int m=0;
                           if(mtrl.parent_id==elm->id) // material is inside this object
                           {
                              if(EqualPath(mtrl_data->src_file,          mtrl_src_file))m=3;else // full path the same - perfect match
                              if(  GetBase(mtrl_data->src_file)==GetBase(mtrl_src_file))m=2;     // base name the same
                           }else
                           {
                              if(EqualPath(mtrl_data->src_file,          mtrl_src_file))m=1; // full path the same, but not inside the object, perhaps this material is used for something else, prefer materials inside the object
                           }
                           if(m>match){match=m; found_mtrl=&mtrl;}
                        }
                     }
                     if(!found_mtrl) // if haven't found then create a new one
                     {
                        found_mtrl=&Proj.newMtrl(src, elm->id, mtrl_src_file);
                        Server.setElmFull(found_mtrl->id);
                     }
                     mtrls.add(found_mtrl->id);
                  }

                  // update mesh
                  mesh_data->newVer();
                  mesh_data->setSrcFile(FileParams::Merge(mesh_data->src_file, import.file), time); // add to file list
                  mesh_data->file_time=time;
                  int pmi=0; FREPD(l, import.mesh.lods()) // set materials
                  {
                     MeshLod &lod=import.mesh.lod(l); FREPA(lod)
                     {
                        if(InRange(pmi, import.part_mtrl_index))
                        {
                           int mtrl_index=import.part_mtrl_index[pmi];
                           if(InRange(mtrl_index, mtrls))lod.parts[i].material(Proj.gamePath(mtrls[mtrl_index]));
                        }
                        pmi++;
                     }
                  }

                  // skeleton and bone mapping
                  Skeleton *mesh_skel, *body_skel; Proj.getMeshSkels(mesh_data, &mesh_skel, &body_skel);
                  if(mesh_skel && import.skel.is())
                  {
                     EditSkeleton edit_skel; if(edit_skel.load(Proj.editPath(mesh_data->skel_id)))
                     {
                        Memt<byte, 256> old_to_new; old_to_new.setNum(import.skel.bones.elms());
                        FREPA(old_to_new) // process from the start
                        {
                           int edit_bone=edit_skel.nodeToBone(edit_skel.findNodeI(import.nodeName(i), import.nodeUID(i))); // find imported bone/node in current edit skeleton
                           if(InRange(edit_bone, edit_skel.bones))
                           {
                              int bone=mesh_skel->findBoneI(edit_skel.bones[edit_bone].name); // find edit bone in mesh skel
                              if(InRange(bone, 256)){old_to_new[i]=bone; goto bone_set;}
                           }

                           {
                              byte parent_index=import.skel.bones[i].parent;
                              old_to_new[i]=(InRange(parent_index, i) ? old_to_new[parent_index] : 0xFF); // set "new bone" as the same as "old parents new bone", use 'i' for range check to check for 'old_to_new' that was already set
                           }
                        bone_set:;
                        }
                        import.mesh.clearSkeleton().boneRemap(old_to_new, false);
                        goto bone_map_set;
                     }
                  }
                  import.mesh.exclude(VTX_SKIN); // if didn't process then remove
               bone_map_set:;

                  Str  edit_path=Proj.editPath(mesh_elm->id);
                  Mesh mesh; Load(mesh, edit_path, Proj.game_path); // load current mesh
                  mesh.skeleton(mesh_skel).skeleton(null); // make sure bone mapping is set
                  mesh.add(import.mesh);
                  Save(mesh, edit_path, Proj.game_path); // save

                  // game mesh
                  Mesh game_mesh; EditToGameMesh(mesh, game_mesh, body_skel, Proj.getEnum(mesh_data->draw_group_id), &NoTemp(mesh_data->transform()));
                  Save(game_mesh, Proj.gamePath(mesh_elm->id)); Proj.savedGame(*mesh_elm); // save
                  mesh_data->from(game_mesh);

                  // notify about change
                              Proj.elmChanged(*     elm);
                  if(mesh_elm)Proj.elmChanged(*mesh_elm);
                //if(skel_elm)Proj.elmChanged(*skel_elm);

                  // send to server
                  Server.setElmFull(obj_data->mesh_id); /*Server.setElmFull(mesh_data.skel_id);*/ Server.setElmShort(elm->id);
               }
               Proj.setList(); // refresh because object may have added new elements
            }
         }break;
      }
   }
   void ImporterClass::update()
   {
      // process imported
      REPA(imports)
      {
         Import &import=imports[i]; if(import.status>=0) // finished importing
         {
            if(!import.cancel)
            {
               if(import.remember_result)*import_results(import.elm_id)=(import.status ? Edit::RELOAD_SUCCESS : Edit::RELOAD_FAILED);
               if(import.status)switch(import.mode) // ok
               {
                  case UPDATE: processUpdate(import); break;
                  case ANIM  : processAnim  (import); break;
                  case CLOTH : processCloth (import); break;
                  case ADD   : processAdd   (import); break;
               }
            }
            imports.removeValid(i);
         }
      }

      // process queue
      for(; import_queue.elms() && imports.elms()<Max(threads.threads()*2, 15); ) // queue 15 elements per frame, so import threads can process before waiting for this code to execute on the next frame
      {
       C ImportElm &import=import_queue.last();
         if(Elm *elm=Proj.findElm(import.elm_id))if(ImportRemovedElms || elm->finalExists())
            importNew(elm->id, elm->parent_id, elm->srcFile(), UPDATE, elm->type, S, import.remember_result);
         import_queue.removeLast();
      }
   }
ImporterClass::ImporterClass() : import_results(Compare), import_target(UIDZero), import_mode(UPDATE) {}

ImporterClass::Import::Import() : status(-1), has_loop(false), cancel(false), remember_result(false), has_color(true), has_alpha(true), ignore_anims(false), type(ELM_NONE), mode(UPDATE), elm_id(UIDZero), parent_id(UIDZero) {}

ImporterClass::Import::ImageEx::ImageEx() : cube(false) {}

/******************************************************************************/
