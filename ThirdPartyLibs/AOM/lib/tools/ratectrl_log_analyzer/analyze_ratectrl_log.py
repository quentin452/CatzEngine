#!/usr/bin/python3
##
## Copyright (c) 2022, Alliance for Open Media. All rights reserved
##
## This source code is subject to the terms of the BSD 2 Clause License and
## the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
## was not distributed with this source code in the LICENSE file, you can
## obtain it at www.aomedia.org/license/software. If the Alliance for Open
## Media Patent License 1.0 was not distributed with this source code in the
## PATENTS file, you can obtain it at www.aomedia.org/license/patent.
##
""" Analyze the log generated by experimental flag CONFIG_RATECTRL_LOG."""

import matplotlib.pyplot as plt
import os


def get_file_basename(filename):
  return filename.split(".")[0]


def parse_log(log_file):
  data_list = []
  with open(log_file) as fp:
    for line in fp:
      dic = {}
      word_ls = line.split()
      i = 0
      while i < len(word_ls):
        dic[word_ls[i]] = float(word_ls[i + 1])
        i += 2
      data_list.append(dic)
    fp.close()
  return data_list


def extract_data(data_list, name):
  arr = []
  for data in data_list:
    arr.append(data[name])
  return arr


def visualize_q_indices(exp_summary, exp_list, fig_path=None):
  for exp in exp_list:
    data = parse_log(exp["log"])
    q_indices = extract_data(data, "q")
    plt.title(exp_summary)
    plt.xlabel("frame_coding_idx")
    plt.ylabel("q_index")
    plt.plot(q_indices, marker=".", label=exp["label"])
  plt.legend()
  if fig_path:
    plt.savefig(fig_path)
  else:
    plt.show()
  plt.clf()


def get_rc_type_from_exp_type(exp_type):
  if exp_type == "Q_3P":
    return "q"
  return "vbr"


def test_video(exe_name, input, exp_type, level, log=None, limit=150):
  basic_cmd = ("--test-decode=warn --threads=0 --profile=0 --min-q=0 --max-q=63"
               " --auto-alt-ref=1 --kf-max-dist=160 --kf-min-dist=0 "
               "--drop-frame=0 --static-thresh=0 --minsection-pct=0 "
               "--maxsection-pct=2000 --arnr-maxframes=7 --arnr-strength=5 "
               "--sharpness=0 --undershoot-pct=100 --overshoot-pct=100 "
               "--frame-parallel=0 --tile-columns=0 --cpu-used=3 "
               "--lag-in-frames=48 --psnr")
  rc_type = get_rc_type_from_exp_type(exp_type)
  rc_cmd = "--end-usage=" + rc_type
  level_cmd = ""
  if rc_type == "q":
    level_cmd += "--cq-level=" + str(level)
  elif rc_type == "vbr":
    level_cmd += "--target-bitrate=" + str(level)
  limit_cmd = "--limit=" + str(limit)
  passes_cmd = "--passes=3 --second-pass-log=second_pass_log"
  output_cmd = "-o test.webm"
  input_cmd = "~/data/" + input
  log_cmd = ""
  if log != None:
    log_cmd = ">" + log
  cmd_ls = [
      exe_name, basic_cmd, rc_cmd, level_cmd, limit_cmd, passes_cmd, output_cmd,
      input_cmd, log_cmd
  ]
  cmd = " ".join(cmd_ls)
  os.system(cmd)


def gen_ratectrl_log(test_case):
  exe = test_case["exe"]
  video = test_case["video"]
  exp_type = test_case["exp_type"]
  level = test_case["level"]
  log = test_case["log"]
  test_video(exe, video, exp_type, level, log=log, limit=150)
  return log


def gen_test_case(exp_type, dataset, videoname, level, log_dir=None):
  test_case = {}
  exe = "./aomenc_bl"
  if exp_type == "BA_3P":
    exe = "./aomenc_ba"
  test_case["exe"] = exe

  video = os.path.join(dataset, videoname)
  test_case["video"] = video
  test_case["exp_type"] = exp_type
  test_case["level"] = level

  video_basename = get_file_basename(videoname)
  log = ".".join([dataset, video_basename, exp_type, str(level)])
  if log_dir != None:
    log = os.path.join(log_dir, log)
  test_case["log"] = log
  return test_case


def run_ratectrl_exp(exp_config):
  fp = open(exp_config)
  log_dir = "./lowres_rc_log"
  fig_dir = "./lowres_rc_fig"
  dataset = "lowres"
  for line in fp:
    word_ls = line.split()
    dataset = word_ls[0]
    videoname = word_ls[1]
    exp_type_ls = ["VBR_3P", "BA_3P"]
    level_ls = [int(v) for v in word_ls[2:4]]
    exp_ls = []
    for i in range(len(exp_type_ls)):
      exp_type = exp_type_ls[i]
      test_case = gen_test_case(exp_type, dataset, videoname, level_ls[i],
                                log_dir)
      log = gen_ratectrl_log(test_case)
      exp = {}
      exp["log"] = log
      exp["label"] = exp_type
      exp_ls.append(exp)
    video_basename = get_file_basename(videoname)
    fig_path = os.path.join(fig_dir, video_basename + ".png")
    visualize_q_indices(video_basename, exp_ls, fig_path)
  fp.close()


if __name__ == "__main__":
  run_ratectrl_exp("exp_rc_config")
