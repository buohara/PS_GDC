%convert a matlab-formatted ps file into a fixed format
%easy to parse in C.

function plot_ps(in_func_str)

	%grab ps function handle from argument
	ps_func=str2func(in_func_str);
	ps=ps_func();
	
	%grab some data from the ps
	buses=ps.bus;
	branches=ps.branch;
	[nbranch dummy]=size(branches);
	
	hold on
	for i=1:nbranch
		plot([buses(branches(i,1),18);buses(branches(i,2),18)],[buses(branches(i,1),19);buses(branches(i,2),19)])
	end
	hold off
end


