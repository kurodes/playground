#! /usr/bin/env python3
#%%
import sys
import json
import re
import html

if len(sys.argv) > 1 and sys.argv[1] == "test":
  TEST = True
else:
  TEST = False

if TEST:
  paper = json.loads("{\"title\":\"Nezha: Deployable and High-Performance Consensus Using Synchronized Clocks. (arXiv:2206.03285v11 [cs.DC] UPDATED)\",\"author\":\" &lt;a href=\\\"http://arxiv.org/find/cs/1/au:+Geng_J/0/1/0/all/0/1\\\"&gt;Jinkun Geng&lt;/a&gt;, &lt;a href=\\\"http://arxiv.org/find/cs/1/au:+Sivaraman_A/0/1/0/all/0/1\\\"&gt;Anirudh Sivaraman&lt;/a&gt;, &lt;a href=\\\"http://arxiv.org/find/cs/1/au:+Prabhakar_B/0/1/0/all/0/1\\\"&gt;Balaji Prabhakar&lt;/a&gt;, &lt;a href=\\\"http://arxiv.org/find/cs/1/au:+Rosenblum_M/0/1/0/all/0/1\\\"&gt;Mendel Rosenblum&lt;/a&gt;\",\"description\":\"&lt;p&gt;This paper presents a high-performance consensus protocol, Nezha, which can\\nbe deployed by cloud tenants without any support from their cloud provider.\\nNezha bridges the gap between protocols such as Multi-Paxos and Raft, which can\\nbe readily deployed and protocols such as NOPaxos and Speculative Paxos, that\\nprovide better performance, but require access to technologies such as\\nprogrammable switches and in-network prioritization, which cloud tenants do not\\nhave. Nezha uses a new multicast primitive called deadline-ordered multicast\\n(DOM). DOM uses high-accuracy software clock synchronization to synchronize\\nsender and receiver clocks. Senders tag messages with deadlines in synchronized\\ntime; receivers process messages in deadline order, on or after their deadline.\\n&lt;/p&gt;\\n&lt;p&gt;We compare Nezha with Multi-Paxos, Fast Paxos, Raft, a NOPaxos version we\\noptimized for the cloud, and 2 recent protocols, Domino and TOQ-EPaxos, that\\nuse synchronized clocks. In throughput, Nezha outperforms all baselines by a\\nmedian of 5.4x (range: 1.9-20.9x). In latency, Nezha outperforms five baselines\\nby a median of 2.3x (range: 1.3-4.0x), with one exception: it sacrifices 33%\\nlatency compared with our optimized NOPaxos in one test. We also prototype two\\napplications, a key-value store and a fair-access stock exchange, on top of\\nNezha to show that Nezha only modestly reduces their performance relative to an\\nunreplicated system. Nezha is available at https://github.com/Steamgjk/Nezha.\\n&lt;/p&gt;\\n\",\"link\":\"http://arxiv.org/abs/2206.03285\"}")
else:
  paper = json.load(sys.stdin)
# print(paper)

title = paper['title'].split(" (arXiv:")[0]
# print(title)

# remove href from the creator string
author = ', '.join(re.findall('[A-Z][a-z]+ [A-Z][a-z]+', paper['author']))
# print(author)

# Decode then remove HTML symbols
description = re.sub(r'<[^>]+>', '', html.unescape(paper['description']))
# remove redundant newlines
description = re.sub(r'\n{1,}', ' ', description.strip())
# print(description)

# %%
import openai
openai.api_key = "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"
message_history = [{"role": "system", "content" : "You are ChatGPT, a large language model trained by OpenAI. Answer as concisely as possible.\nKnowledge cutoff: 2021-09-01\nCurrent date: 2023-03-02"}]
def chat(inp, role = "user"):
  message_history.append({"role": role, "content": inp})
  completion = openai.ChatCompletion.create(
    model= "gpt-3.5-turbo",
    # model= "gpt-4",
    messages=message_history
  )
  reply_content = completion.choices[0].message.content
  message_history.append({"role": "assistant", "content": reply_content})
  return reply_content

translation = chat(f"Translate this abstract of a computer science paper into Chinese: {description}")
if TEST:
  print(translation)
background = chat(f"Explain the background concepts related to this paper.")
if TEST:
  print(background)
# translation = "This is translation"
# background = "This is background"



# %%
rss_item = {}
rss_item["title"] = title
background_lines = re.sub(r'\n', '<br>', background)
rss_item["description"] = f"<p>{author}</p><p>{description}</p><p>{translation}</p><p>{background}</p>"
rss_item["link"] = paper["link"]
print(json.dumps(rss_item))