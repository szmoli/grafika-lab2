inc_dir := inc		# directory for the header files
src_dir := src		# directory for the source files
out_dir := out		# directory for the executable
target := app.out 	# name of the executable
libs := glfw		# libraries to link
lib_flags := $(addprefix -l,$(libs))
inc_flags := $(addprefix -I,$(inc_dir))

# default rule: build the target
$(out_dir)/$(target): $(wildcard $(src_dir)/*.c*)
	mkdir -p $(out_dir)
	g++ $(inc_flags) $^ $(lib_flags) -o $(out_dir)/$(target)

# run the target, if it doesn't exist build it first
run: $(out_dir)/$(target)
	$(out_dir)/$(target)

# delete the out directory
clean:
	rm -r $(out_dir)