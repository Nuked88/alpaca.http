# Alpaca.http

Start a fast ChatGPT-like model on your pc and interact remotely with it via HTTP requests

This combines the [LLaMA foundation model](https://github.com/facebookresearch/llama) with an [open reproduction](https://github.com/tloen/alpaca-lora) of [Stanford Alpaca](https://github.com/tatsu-lab/stanford_alpaca) a fine-tuning of the base model to obey instructions (akin to the [RLHF](https://huggingface.co/blog/rlhf) used to train ChatGPT) and a set of modifications to [alpaca.cpp]( https://github.com/antimatter15/alpaca.cpp) to replace the chat interface with an http """API""" interface (only the parameter **text** for now).

No performance seems to have been lost, the result is not streamed so it may look like it will take longer but it really isn't.

## Get Started (7B)


Download the zip file corresponding to your operating system from the [latest release](https://github.com/nuked88/alpaca.http/releases/latest). No MacOS release because i dont have a dev key :(  But you can still build it from source! 

Download `ggml-alpaca-7b-q4.bin` and place it in the same folder as the `server` executable in the zip file. I found this urls that should work: 

#### Alpaca-7b Q4
https://ipfs.io/ipfs/QmUp1UGeQFDqJKvtjbSYPBiZZKRjLp8shVP9hT8ZB9Ynv1

### Alpaca-13b Q4
magnet:?xt=urn:btih:053b3d54d2e77ff020ebddf51dad681f2a651071&dn=ggml-alpaca-13b-q4.bin&tr=udp%3a%2f%2ftracker.opentrackr.org%3a1337%2fannounce&tr=udp%3a%2f%2fopentracker.i2p.rocks%3a6969%2fannounce&tr=udp%3a%2f%2ftracker.openbittorrent.com%3a6969%2fannounce&tr=udp%3a%2f%2f9.rarbg.com%3a2810%2fannounce

### Alpaca-30b Q4 (not tested)
magnet:?xt=urn:btih:6K5O4J7DCKAMMMAJHWXQU72OYFXPZQJG&dn=ggml-alpaca-30b-q4.bin&xl=20333638921&tr=udp%3A%2F%2Ftracker.openbittorrent.com%3A80%2Fannounce


Once you've downloaded the model weights and placed them into the same directory as the `server` or `server.exe` executable, run:

```
./server
```

If you want to load another model just use the **-m** argument

### Command line arguments added
```
-sp or --server-port     change the default 8080 server port
-sa or --server-address     change the default 0.0.0.0 server address
```


The weights are based on the published fine-tunes from `alpaca-lora`, converted back into a pytorch checkpoint with a [modified script](https://github.com/tloen/alpaca-lora/pull/19) and then quantized with llama.cpp the regular way. 

## Using Alpaca.http to interact with the GGML Alpaca model

#### Note: Alpaca.http does not handle conversations
It's important to note that Alpaca.http does not handle conversations. This means that you'll need to manage conversations separately in your program. Alpaca.http simply sends requests to the GGML Alpaca model and returns the responses.

#### Making requests to the server
To interact with the server, If you're using GET, you can simply include the prompt in the URL with the **text** parameter. If you're using POST, you'll need to send a raw request with the prompt in the body.

##### See the [examples](https://github.com/Nuked88/alpaca.http/tree/master/examples) directory

#### Response format
The server will return a JSON object with the following fields:

* **created**: The timestamp of when the response was created.
* **model**: The name of the model used to generate the response.
* **output**: The generated text, followed by an [end of text] marker.
* **usage**: An object containing information about how many tokens were used to generate the response.


Here's an example response:

```
{
    "created": 16798385514,
    "model": "ggml-alpaca-13b-q4.bin",
    "output": "Alpacas are members of the camel family and originated in South America, particularly Peru where they were domesticated by indigenous people over 600 years ago for their wool. They have a thick coat that is used to make sweaters or other clothing items such as hats, gloves etc., which are very popular with tourists visiting the country and also in high demand internationally due to its softness and warmth. Alpacas can be found on farms around Peru where they live together peacefully alongside llamas who help protect them from predators such as foxes, coyotes etc., which are common throughout South America\n[end of text]\n",
    "usage": {
        "completion_tokens": 141,
        "prompt_tokens": 8,
        "total_tokens": 149
    }
}
```

The **usage** object includes the following fields:

* **completion_tokens**: The number of tokens used to generate the response.
* **prompt_tokens**: The number of tokens in the prompt.
* **total_tokens**: The total number of tokens used, including the prompt tokens.





## Build Requirements

To use this library, you need to have Boost installed on your system.

### Installing Boost

1. Download the Boost library from https://www.boost.org/users/history/version_1_81_0.html
2. Extract the `boost` folder from the downloaded archive
3. Place the `boost` folder in the root directory of the cloned repository

## Building from Source (MacOS/Linux)


```sh
git clone https://github.com/Nuked88/alpaca.http.git
cd alpaca.cpp

make server
./server
```


## Building from Source (Windows)

- Download and install CMake: <https://cmake.org/download/>
- Download and install `git`. If you've never used git before, consider a GUI client like <https://desktop.github.com/>
- Clone this repo using your git client of choice (for GitHub Desktop, go to File -> Clone repository -> From URL and paste `https://github.com/antimatter15/alpaca.cpp` in as the URL)
- Open a Windows Terminal inside the folder you cloned the repository to
- Run the following commands one by one:

```ps1
cmake .
cmake --build . --config Release
```

- Download the weights via any of the links in "Get started" above, and save the file as `ggml-alpaca-7b-q4.bin` in the main Alpaca directory.
- In the terminal window, run this command:
```ps1
.\Release\server.exe
```
- (You can add other launch options like `--n 8` as preferred onto the same line)


## Disclaimer

Please note that I am not an expert in C++ and that there may be bugs or gross errors in this library. This library was created with the goal of providing a server that can be interfaced with various programming languages, so even those who are not experts like myself can fully utilize the Alpaca models.

If you encounter any issues or have any suggestions, please feel free to open an issue on the GitHub repository. I will do my best to address them in a timely manner. Thank you for your understanding and support.

## Credit

This combines [Facebook's LLaMA](https://github.com/facebookresearch/llama), [Stanford Alpaca](https://crfm.stanford.edu/2023/03/13/alpaca.html), [alpaca-lora](https://github.com/tloen/alpaca-lora) and [corresponding weights](https://huggingface.co/tloen/alpaca-lora-7b/tree/main) by Eric Wang (which uses [Jason Phang's implementation of LLaMA](https://github.com/huggingface/transformers/pull/21955) on top of Hugging Face Transformers), and [llama.cpp](https://github.com/ggerganov/llama.cpp) by Georgi Gerganov. Inspired by [Simon Willison's](https://til.simonwillison.net/llms/llama-7b-m2) getting started guide for LLaMA. [Andy Matuschak](https://twitter.com/andy_matuschak/status/1636769182066053120)'s thread on adapting this to 13B, using fine tuning weights by [Sam Witteveen](https://huggingface.co/samwit/alpaca13B-lora). 
Library used: [BOOST](https://www.boost.org/).Nlohmann-json library (https://github.com/nlohmann/json), distributed under the MIT License.


## Disclaimer

Note that the model weights are only to be used for research purposes, as they are derivative of LLaMA, and uses the published instruction data from the Stanford Alpaca project which is generated by OpenAI, which itself disallows the usage of its outputs to train competing models. 


