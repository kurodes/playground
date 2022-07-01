#!/usr/bin/env python3

import subprocess
import shlex

write_req_cmd = "sudo ipmctl show -dimm -performance TotalWriteRequests"
media_write_cmd = "sudo ipmctl show -dimm -performance TotalMediaWrites"
read_req_cmd = "sudo ipmctl show -dimm -performance TotalReadRequests"
media_read_cmd = "sudo ipmctl show -dimm -performance TotalMediaReads"

def get_perf_data(cmd):
    res_write = subprocess.check_output(shlex.split(cmd))
    res_write = res_write.split()
    return res_write

def extract_device_name(s):
    temp = s.split(b"---")
    temp = temp[1].split(b"=")
    return temp[1]

def extract_number(s):
    s = s.split(b"=")
    total_reqs = int(s[1],16)
    return total_reqs

def res_to_map(cmd):
    res = get_perf_data(cmd)
    m = {}
    cur_key = ""
    for (i,l) in enumerate(res):
        if i % 2 == 0 :
            cur_key = extract_device_name(l)
        else:
            m[cur_key] = extract_number(l)
    return m


def report_delta(p,n,name,time,threshold, report):
    delta_res = {}
    for k in n:
        delta = n[k] - p[k]
        if delta >= threshold:
            thpt = delta / time
            bd   = thpt * 64 / (1024 * 1024) ## in MB
            #print("device: %s bd: %.2f MB" % (k,bd))
            if k in report:
                report[k].append("%s bd: %.2f MB" % (name,bd))
            else:
                report[k] = ["%s bd: %.2f MB" % (name,bd)]
            delta_res[k] = thpt
    return delta_res

writes = res_to_map(write_req_cmd)
media_writes = res_to_map(media_write_cmd)
reads    = res_to_map(read_req_cmd)
media_reads = res_to_map(media_read_cmd)

import time
start_w = time.time() ## start time of the write request
start_m = time.time() ## start time of the media write requests
start_r = time.time() ## start time of the read request
start_mr = time.time() ## start time of the media read requests

epoch = 0
while True:
    time.sleep(1)

    report = {}
    threshold = 100000 ## 6.4MB/s
    # threshold = 0

    end = time.time()
    temp = res_to_map(write_req_cmd)
    write_reqs = report_delta(writes, temp, "*write req*",end - start_w, threshold, report)
    writes = temp
    start_w = end

    end = time.time()
    temp = res_to_map(media_write_cmd)
    write_medias = report_delta(media_writes, temp, "*media write*", end - start_m, threshold, report)
    media_writes = temp
    start_m = end

    end = time.time()
    temp = res_to_map(read_req_cmd)
    read_reqs = report_delta(reads, temp, "*read req*", end - start_r, threshold, report)
    reads = temp
    start_r = end

    end = time.time()
    temp = res_to_map(media_read_cmd)
    read_medias = report_delta(media_reads, temp, "*media read*", end - start_mr, threshold, report)
    media_reads = temp
    start_mr = end

    for k in report:
        print ("device %s: " % k, report[k])

    for k in write_reqs:
        try :
            wm = write_medias[k]
            wr = write_reqs[k]
            rm = read_medias[k]
            rr = read_reqs[k]
            print("write amp for %s: %f" % (k, float(wm) / wr))
            print("read amp for %s: %f" % (k, float(rm) / rr))
        except:
            pass

    print ("------epoch done---------",epoch)
    epoch += 1
