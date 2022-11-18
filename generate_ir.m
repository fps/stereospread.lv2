rng(0);

% [audio, samplerate] = audioread("clean_marshall_riff.wav");

f = fopen('ir.c', 'w');

lengths = [5 10 20 40 80 160 320 640 1280];

for filter_length = lengths

    % filter_length = 20;

    random_vector = rand(filter_length,1);

    % filter_length = 1000;

    spread = pi/4;

    phases = spread*random_vector(1:filter_length) - spread/2;
    amplitudes = 1./cos(phases);

    hfft1 = amplitudes .* exp(-1i*phases);
    hfft2 = conj(hfft1);

    % hfft1 = 1+2*rand(100,1)-0.5
    % hfft2 = 1./hfft1

    f1 = ifft([1; hfft1; conj(hfft1((end-1):-1:1))], 'symmetric');
    f2 = ifft([1; hfft2; conj(hfft2((end-1):-1:1))], 'symmetric');

    fprintf(f, sprintf("const float ir_%d[] = {\n", filter_length*2));

    for index = 1:(filter_length*2)
        fprintf(f, "    %1.16f, %1.16f", f1(index), f2(index));
        if index ~= (filter_length*2)
            fprintf(f, ",");
        end
        fprintf(f, "\n");
    end

    fprintf(f, "};\n");
end

fprintf(f, "const float* irs[] = {\n");
for filter_length = lengths
    fprintf(f, sprintf("    ir_%d", 2*filter_length));
    if filter_length ~= lengths(end)
        fprintf(f, ",");
    end
    fprintf(f,"\n");
end
fprintf(f, "};\n");

fprintf(f, "const int ir_lengths[] = {\n");
for filter_length = lengths
    fprintf(f, "    %d", 2*filter_length);
    if filter_length ~= lengths(end)
        fprintf(f, ",");
    end
    fprintf(f,"\n");
end
fprintf(f, "};\n");

fclose(f);
