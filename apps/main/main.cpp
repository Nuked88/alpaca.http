#include "common.h"
#include "llama.h"

#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <signal.h>
#include <unistd.h>
#elif defined (_WIN32)
#include <signal.h>
#endif

// Boost Beast library
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <iostream>

/* 
This project uses the nlohmann-json library (https://github.com/nlohmann/json), distributed under the MIT License.
MIT License 

Copyright (c) 2013-2022 Niels Lohmann

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
*/
// Niels Lohmann library 
#include <nlohmann/json.hpp>


#if defined (_WIN32)
#pragma comment(lib,"kernel32.lib")
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(unsigned long nStdHandle);
extern "C" __declspec(dllimport) int __stdcall GetConsoleMode(void* hConsoleHandle, unsigned long* lpMode);
extern "C" __declspec(dllimport) int __stdcall SetConsoleMode(void* hConsoleHandle, unsigned long dwMode);
extern "C" __declspec(dllimport) int __stdcall SetConsoleCP(unsigned int wCodePageID);
extern "C" __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int wCodePageID);
#endif

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_BOLD          "\x1b[1m"

/* Keep track of current color of output, and emit ANSI code if it changes. */
enum console_state {
    CONSOLE_STATE_DEFAULT=0,
    CONSOLE_STATE_PROMPT,
    CONSOLE_STATE_USER_INPUT
};

static console_state con_st = CONSOLE_STATE_DEFAULT;
static bool con_use_color = false;
using tcp = boost::asio::ip::tcp; // alias IP


void set_console_state(console_state new_st) {
    if (!con_use_color) return;
    // only emit color code if state changed
    if (new_st != con_st) {
        con_st = new_st;
        switch(con_st) {
        case CONSOLE_STATE_DEFAULT:
            printf(ANSI_COLOR_RESET);
            return;
        case CONSOLE_STATE_PROMPT:
            printf(ANSI_COLOR_YELLOW);
            return;
        case CONSOLE_STATE_USER_INPUT:
            printf(ANSI_BOLD ANSI_COLOR_GREEN);
            return;
        }
    }
}

static bool is_interacting = false;

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) || defined (_WIN32)
void sigint_handler(int signo) {
    set_console_state(CONSOLE_STATE_DEFAULT);
    printf("\n"); // this also force flush stdout.
    if (signo == SIGINT) {
        if (!is_interacting) {
            is_interacting=true;
        } else {
            _exit(130);
        }
    }
}
#endif

#if defined (_WIN32)
void win32_console_init(void) {
    unsigned long dwMode = 0;
    void* hConOut = GetStdHandle((unsigned long)-11); // STD_OUTPUT_HANDLE (-11)
    if (!hConOut || hConOut == (void*)-1 || !GetConsoleMode(hConOut, &dwMode)) {
        hConOut = GetStdHandle((unsigned long)-12); // STD_ERROR_HANDLE (-12)
        if (hConOut && (hConOut == (void*)-1 || !GetConsoleMode(hConOut, &dwMode))) {
            hConOut = 0;
        }
    }
    if (hConOut) {
        // Enable ANSI colors on Windows 10+
        if (con_use_color && !(dwMode & 0x4)) {
            SetConsoleMode(hConOut, dwMode | 0x4); // ENABLE_VIRTUAL_TERMINAL_PROCESSING (0x4)
        }
        // Set console output codepage to UTF8
        SetConsoleOutputCP(65001); // CP_UTF8
    }
    void* hConIn = GetStdHandle((unsigned long)-10); // STD_INPUT_HANDLE (-10)
    if (hConIn && hConIn != (void*)-1 && GetConsoleMode(hConIn, &dwMode)) {
        // Set console input codepage to UTF8
        SetConsoleCP(65001); // CP_UTF8
    }
}
#endif



std::string url_decode(const std::string& str) {
    std::string result;
    char c;
    int i = 0, len = str.length();
    while (i < len) {
        c = str[i];
        if (c == '+') {
            result += ' ';
        }
        else if (c == '%') {
            char hex[3];
            hex[0] = str[++i];
            hex[1] = str[++i];
            hex[2] = '\0';
            result += strtol(hex, nullptr, 16);
        }
        else {
            result += c;
        }
        i++;
    }
    return result;
}
/********************************/
/**************MAIN**************/
/********************************/

int main(int argc, char ** argv) {
    gpt_params params;
    params.model = "models/llama-7B/ggml-model.bin";

    if (gpt_params_parse(argc, argv, params) == false) {
        return 1;
    }

    // server init
    boost::asio::io_context io_context(1); // I/O context creation
    tcp::endpoint endpoint(boost::asio::ip::make_address(params.server_address), params.server_port); // Endpoint TCP creation tcp::v4() 
    tcp::acceptor acceptor(io_context, endpoint); // Socket accteptor creation
    std::string address = endpoint.address().to_string();
    int port = endpoint.port();
    printf(ANSI_COLOR_GREEN);
    printf("Server started on %s:%d\n", address.c_str(), port);
    printf(ANSI_COLOR_RESET);

    // save choice to use color for later
    // (note for later: this is a slightly awkward choice)
    con_use_color = params.use_color;

    #if defined (_WIN32)
        win32_console_init();
    #endif

    if (params.perplexity) {
        printf("\n************\n");
        printf("%s: please use the 'perplexity' tool for perplexity calculations\n", __func__);
        printf("************\n\n");

        return 0;
    }

    if (params.embedding) {
        printf("\n************\n");
        printf("%s: please use the 'embedding' tool for embedding calculations\n", __func__);
        printf("************\n\n");

        return 0;
    }

    if (params.n_ctx > 2048) {
        fprintf(stderr, "%s: warning: model does not support context sizes greater than 2048 tokens (%d specified);"
                "expect poor results\n", __func__, params.n_ctx);
    }

    if (params.seed <= 0) {
        params.seed = time(NULL);
    }

    fprintf(stderr, "%s: seed = %d\n", __func__, params.seed);

    std::mt19937 rng(params.seed);
    if (params.random_prompt) {
        params.prompt = gpt_random_prompt(rng);
    }

    //    params.prompt = R"(// this function checks if the number n is prime
    //bool is_prime(int n) {)";

    llama_context * ctx;

    // load the model
    {
        auto lparams = llama_context_default_params();

        lparams.n_ctx      = params.n_ctx;
        lparams.n_parts    = params.n_parts;
        lparams.seed       = params.seed;
        lparams.f16_kv     = params.memory_f16;
        lparams.use_mlock  = params.use_mlock;

        ctx = llama_init_from_file(params.model.c_str(), lparams);

        if (ctx == NULL) {
            fprintf(stderr, "%s: error: failed to load model '%s'\n", __func__, params.model.c_str());
            return 1;
        }
    }

    // print system information
    {
        fprintf(stderr, "\n");
        fprintf(stderr, "system_info: n_threads = %d / %d | %s\n",
                params.n_threads, std::thread::hardware_concurrency(), llama_print_system_info());
    }

    // determine the maximum memory usage needed to do inference for the given n_batch and n_predict parameters
    // uncomment the "used_mem" line in llama.cpp to see the results
    if (params.mem_test) {
        {
            const std::vector<llama_token> tmp(params.n_batch, 0);
            llama_eval(ctx, tmp.data(), tmp.size(), 0, params.n_threads);
        }

        {
            const std::vector<llama_token> tmp = { 0, };
            llama_eval(ctx, tmp.data(), tmp.size(), params.n_predict - 1, params.n_threads);
        }

        llama_print_timings(ctx);
        llama_free(ctx);

        return 0;
    }

    // Add a space in front of the first character to match OG llama tokenizer behavior
    params.prompt.insert(0, 1, ' ');

    // tokenize the prompt
    auto embd_inp = ::llama_tokenize(ctx, params.prompt, true);

    const int n_ctx = llama_n_ctx(ctx);

    if ((int) embd_inp.size() > n_ctx - 4) {
        fprintf(stderr, "%s: error: prompt is too long (%d tokens, max %d)\n", __func__, (int) embd_inp.size(), n_ctx - 4);
        return 1;
    }

    params.n_keep    = std::min(params.n_keep,    (int) embd_inp.size());

    // prefix & suffix for instruct mode
    const auto inp_pfx = ::llama_tokenize(ctx, "\n\n### Instruction:\n\n", true);
    const auto inp_sfx = ::llama_tokenize(ctx, "\n\n### Response:\n\n", false);

    // in instruct mode, we inject a prefix and a suffix to each input by the user
    
    params.interactive = false;
    params.antiprompt.push_back("### Instruction:\n\n");
    


    // determine newline token
    auto llama_token_newline = ::llama_tokenize(ctx, "\n", false);

    if (params.verbose_prompt) {
        fprintf(stderr, "\n");
        fprintf(stderr, "%s: prompt: '%s'\n", __func__, params.prompt.c_str());
        fprintf(stderr, "%s: number of tokens in prompt = %zu\n", __func__, embd_inp.size());
        for (int i = 0; i < (int) embd_inp.size(); i++) {
            fprintf(stderr, "%6d -> '%s'\n", embd_inp[i], llama_token_to_str(ctx, embd_inp[i]));
        }
        if (params.n_keep > 0) {
        fprintf(stderr, "%s: static prompt based on n_keep: '", __func__);
            for (int i = 0; i < params.n_keep; i++) {
                fprintf(stderr, "%s", llama_token_to_str(ctx, embd_inp[i]));
            }
            fprintf(stderr, "'\n");
        }
        fprintf(stderr, "\n");
    }

   
    fprintf(stderr, "sampling: temp = %f, top_k = %d, top_p = %f, repeat_last_n = %i, repeat_penalty = %f\n", params.temp, params.top_k, params.top_p, params.repeat_last_n, params.repeat_penalty);
    fprintf(stderr, "generate: n_ctx = %d, n_batch = %d, n_predict = %d, n_keep = %d\n", n_ctx, params.n_batch, params.n_predict, params.n_keep);
    fprintf(stderr, "\n\n");

    // TODO: replace with ring-buffer
    std::vector<llama_token> last_n_tokens(n_ctx);
    std::fill(last_n_tokens.begin(), last_n_tokens.end(), 0);

    
    //std::vector<gpt_vocab::id> instruct_inp = ::llama_tokenize(vocab, " Below is an instruction that describes a task. Write a response that appropriately completes the request.\n\n", true);
    //std::vector<gpt_vocab::id> prompt_inp = ::llama_tokenize(vocab, "### Instruction:\n\n", true);
    //std::vector<gpt_vocab::id> response_inp = ::llama_tokenize(vocab, "### Response:\n\n", false);

    bool input_noecho = false;
    
    int32_t completion_tokens = 0;
    int32_t  prompt_tokens = 0;






 while (true) {
    int n_past     = 0;
    int n_remain   = params.n_predict;
    int n_consumed = 0;


    
     //std::vector<gpt_vocab::id> embd_inp;
     std::string output;
     std::string text;
     tcp::socket socket(io_context);  // socket creation
     acceptor.accept(socket); // accept connection
     boost::beast::flat_buffer buffer; // buffer for received data
     boost::beast::http::request<boost::beast::http::string_body> request; // HTTP request
     boost::beast::http::read(socket, buffer, request); // read request


    using namespace boost::beast::http;
           if (request.method() == verb::post) {
        // read the raw request body from POST request
            text = request.body().data();
            is_interacting = true;
            //printf("Received %s", text);
        } else if (request.method() == verb::get) {
            // get query string from GET request
            std::string query = std::string(request.target());
            
            if (auto pos = query.find("?text="); pos != std::string::npos) {
                auto query_string = query.substr(pos + 6);
                text = url_decode(query_string);
                is_interacting = true;
            }
        }

    if (text.size() > 0) {
       
               
        auto line_inp = ::llama_tokenize(ctx, text, false);
        prompt_tokens = line_inp.size();
        embd_inp.insert(embd_inp.end(), line_inp.begin(), line_inp.end());
        embd_inp.insert(embd_inp.end(), inp_sfx.begin(), inp_sfx.end());
        //embd_inp.insert(embd_inp.end(), prompt_inp.begin(), prompt_inp.end());
        //embd_inp.insert(embd_inp.end(), param_inp.begin(), param_inp.end());
        //embd_inp.insert(embd_inp.end(), response_istd::vector<gpt_vocab::id> embd;np.begin(), response_inp.end());
                    
        bool input_noecho = true;
        

        // the first thing we will do is to output the prompt, so set color accordingly
        set_console_state(CONSOLE_STATE_PROMPT);
        std::vector<llama_token> embd;

    while (n_remain != 0) {
        // predict
        
        
        if (embd.size() > 0) {


            if (llama_eval(ctx, embd.data(), embd.size(), n_past, params.n_threads)) {
                fprintf(stderr, "%s : failed to eval\n", __func__);
                return 1;
            }
        }

        n_past += embd.size();
        embd.clear();

        if ((int) embd_inp.size() <= n_consumed) {
            // out of user input, sample next token
            const float top_k          = params.top_k;
            const float top_p          = params.top_p;
            const float temp           = params.temp;
            const float repeat_penalty = params.repeat_penalty;

            llama_token id = 0;
            {
                auto logits = llama_get_logits(ctx);
                if (params.ignore_eos) {
                    logits[llama_token_eos()] = 0;
                }
                id = llama_sample_top_p_top_k(ctx,
                        last_n_tokens.data() + n_ctx - params.repeat_last_n,
                        params.repeat_last_n, top_k, top_p, temp, repeat_penalty);
                last_n_tokens.erase(last_n_tokens.begin());
                last_n_tokens.push_back(id);
            }

            // replace end of text token with newline token when in interactive mode

            if (id == llama_token_eos() ) {
                
                break;
                
            }
            // add it to the context
            embd.push_back(id);
            // echo this to console
            input_noecho = false;
            // decrement remaining sampling budget
            --n_remain;
        } else {
            // some user input remains from prompt or interaction, forward it to processing
            while ((int) embd_inp.size() > n_consumed) {
                embd.push_back(embd_inp[n_consumed]);
                last_n_tokens.erase(last_n_tokens.begin());
                last_n_tokens.push_back(embd_inp[n_consumed]);
                ++n_consumed;
                if ((int) embd.size() >= params.n_batch) {
                    break;
                }
            }
        }

        // display text
        if (!input_noecho) {
            for (auto id : embd) {
                std::string current_p = llama_token_to_str(ctx, id);
                output += current_p;
                completion_tokens++;
                printf("%s", current_p);
            }
            fflush(stdout);
        }
       
       /*****************************************************************/

     
                
                

        // end of text token
        if (embd.back() == llama_token_eos()) {
          
                break;
           
        }

        // In interactive mode, respect the maximum number of tokens and drop back to user input when reached.
        if (n_remain <= 0 && params.n_predict != -1) {
            n_remain = params.n_predict;
           
        }
    }



                // build the JSON response

                
                auto timestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();

                nlohmann::json json_response;
                json_response["created"] = timestamp;
                json_response["model"] = params.model;
                json_response["output"] = output;
                json_response["usage"]["prompt_tokens"] = prompt_tokens;
                json_response["usage"]["completion_tokens"] = completion_tokens;
                json_response["usage"]["total_tokens"] = completion_tokens + prompt_tokens;


                // HTTP
                boost::beast::http::response<boost::beast::http::string_body> response;
                response.version(request.version());
                response.result(boost::beast::http::status::ok);
                response.set(boost::beast::http::field::server, "Alpaca.http server");
                response.set(boost::beast::http::field::content_type, "application/json");
                response.keep_alive(request.keep_alive());
                response.body() =  json_response.dump();
                response.prepare_payload();

                boost::beast::http::write(socket, response); // send

    
    #if defined (_WIN32)
        signal(SIGINT, SIG_DFL);
    #endif

    llama_print_timings(ctx);
 }}
    llama_free(ctx);

    set_console_state(CONSOLE_STATE_DEFAULT);

    return 0;
}
