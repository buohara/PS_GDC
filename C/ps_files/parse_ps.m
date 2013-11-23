%convert a matlab-formatted ps file into a fixed format
%easy to parse in C.

function parse_ps(in_func_str,out_file_str)

	%grab ps function handle from argument
	ps_func=str2func(in_func_str);
	ps=ps_func();
	
	%define output format for various ps elements
	bus_fmt="%d %d %g %g %g %g %d %g %g %g %d %g %g %g %g %g %g %g %g\n";
	branch_fmt="%d %d %g %g %g %g %g %g %d %d %d\n";
	gen_fmt="%d %g %g %g %g %g %g %d %g %g\n";
	shunt_fmt="%d %g %g %g %g\n";
	
	%grab some data from the ps
	base_mva=ps.baseMVA;
	buses=ps.bus;
	branches=ps.branch;
	gens=ps.gen;
	shunts=ps.shunt;
	
	%grab numbers of various ps elements
	[nbus dummy]=size(buses);	
	[nbranch dummy]=size(branches);
	[ngen dummy]=size(gens);
	[nshunt dummy]=size(shunts);
	
	%create the output file
	out_file=fopen(out_file_str,'w');
	
	%write out all the ps data to file
	
	%base_mva
	fprintf(out_file,"BASE_MVA %g\n",base_mva);	
	
	%buses
	fprintf(out_file,"BUS %g\n",nbus);
	for i=1:nbus
		fprintf(out_file,bus_fmt,buses(i,1:19));
	end
	
	%branches
	fprintf(out_file,"BRANCH %g\n",nbranch);
	for i=1:nbranch
		fprintf(out_file,branch_fmt,branches(i,1:11));
	end
	
	%generators
	fprintf(out_file,"GEN %g\n",ngen);
	for i=1:ngen
		fprintf(out_file,gen_fmt,gens(i,1:10));
	end
	
	%shunts
	fprintf(out_file,"SHUNT %g\n",nshunt);
	for i=1:nshunt
		fprintf(out_file,shunt_fmt,shunts(i,1:5));
	end
	
	%close and done
	fclose(out_file);

end

parse_ps("case6_ps","case6_ps.psc");
parse_ps("case30_ps","case30_ps.psc");
parse_ps("case300_001_ps","case300_ps.psc");
parse_ps('case2383_mod_ps','case2383_ps.psc');

