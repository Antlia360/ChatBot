import time
import torch
import sys
from transformers import AutoTokenizer, AutoModelForCausalLM

def generate_text(prompt,user_id):
    try:
        # start = time.time()
        # Load pre-trained GPT-2 model and tokenizer
        #model_name = "gpt2"
        model_name = "openai-community/gpt2" 
        tokenizer = AutoTokenizer.from_pretrained(model_name)
        model = AutoModelForCausalLM.from_pretrained(model_name)

        # Set device to GPU if available
        device = "cuda" if torch.cuda.is_available() else "cpu"
        model.to(device)
        # print(device)

        # Generate text based on a prompt (same generate_text() function as before)
        max_length=200
        input_ids = tokenizer.encode(prompt, return_tensors="pt").to(device)
        output = model.generate(input_ids, max_new_tokens = 40, pad_token_id=tokenizer.eos_token_id, no_repeat_ngram_size=3, num_return_sequences=1, top_p = 0.95, temperature = 0.2, do_sample = True)
        generated_text = tokenizer.decode(output[0], skip_special_tokens=True)
        # end = time.time()
        # print(f'Time taken = {end - start}')
        # # Print the generated text
        # print("Generated Text:")
        
        output_file = f"{user_id}.txt"
        with open(output_file, "w") as file:
            file.write(generated_text)
        file.close()
        # return generated_text
    except Exception as e:
        # Handle any exceptions that occur during the process
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        # print("Usage: python3 gpt_2_gen.py <prompt>")
        sys.exit(1)
    prompt = sys.argv[1]
    user_id = sys.argv[2]
    generate_text(prompt, user_id)
    
    # with open("output.txt", "w") as file:
    #     file.write(generate_text(prompt))
    # file.close()
    # print(generate_text(prompt))