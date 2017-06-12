namespace irr_map{


vecf4 irr(triangle* prim, float u, float v){
	const float w = 1.0f - u - v;

	pos =
		prim->v0.pos * w +
		prim->v1.pos * u +
		prim->v2.pos * v;

	n = prim->v0.n * w +
		prim->v1.n * u +
		prim->v2.n * v;

	vecf4::normalize(n);

	uint sample_count;
	return est_irr_adap(pos, n, sample_count);
}

void gen_irr_map(){
	const float rel_err_eps = 0.0001f;
	const float border_eps = 0.00001f;
	const float rel_err_bound = ctrlWnd->irrmap_eps_dsb->value();


	const float samples_on_edge = ctrlWnd->sample_diag_dsb->value() * prim->edge / bvh->edge_mean;
	const float cur_max_order = samples_on_edge+1.0f;
	const uint cur_max_exp = max(ceilf(log2f(cur_max_order)), 1.0f);

	// const uint max_exp = 7;
	const uint read_max_exp = ctrlWnd->irrmap_max_order_sb->value();
	const uint max_exp = (cur_max_exp < read_max_exp)?(cur_max_exp):(read_max_exp);
	uint max_order = 1 << max_exp;
	const uint half_max_order = max_order >> 1;
	const uint max_size = size(max_order);
	const uint half_max_size = size(half_max_order);

	// memory indices of vertices
	const uint a = 0;
	const uint b = max_size-max_order-1;
	const uint c = max_size-1;

	// memory indices of half edge samples
	const uint eab = half_max_size-half_max_order-1;
	const uint ebc = max_size-half_max_order-1;
	const uint eca = half_max_size-1;


	vecf4 data[max_size];
	triangle* prim;

	while(prim = next_triangle()){
		uint cur_exp = 1;
		uint cur_order = 1 << cur_exp;

		// vertices
		data[a] = irr(prim, border_eps, border_eps);
		data[c] = irr(prim, border_eps, 1.0f - 2.0f * border_eps);
		data[b] = irr(prim, 1.0f - 2.0f*border_eps, border_eps);

		// edges
		data[eca] = irr(prim, border_eps, 0.5f);
		data[eab] = irr(prim, 0.5f, border_eps);
		data[ebc] = irr(prim, 0.5f - border_eps, 0.5f - border_eps);

		float rel_err = max(fabsf( 0.5f*(data[a] + data[b]) - data[eab] ) / (rel_err_eps + data[eab]));
		rel_err = max(rel_err, max(fabsf( 0.5f*(data[b] + data[c]) - data[ebc] ) / (rel_err_eps + data[ebc])));
		rel_err = max(rel_err, max(fabsf( 0.5f*(data[c] + data[a]) - data[eca] ) / (rel_err_eps + data[eca])));


		while (rel_err > rel_err_bound && cur_exp < max_exp){
			cur_exp++;
			cur_order = 1 << cur_exp;
			float inv_cur_order = 1.0f / float(cur_order);

	  		// compute new samples and residual norm
			const uint shift = max_exp - cur_exp;

	  		// edges
			for (uint i = 1; i < cur_order; i+=2){
				const float tmp = float(i) * inv_cur_order;

				uint idx = memidx(i,0,shift);
				data[idx] = irr(prim, tmp, border_eps);

				idx = memidx(0,i,shift);
				data[idx] = irr(prim, border_eps, tmp);

				idx = memidx(i,cur_order-i,shift);
				data[idx] = irr(prim, tmp-border_eps, 1.0f-tmp-border_eps);

				rel_err = max(rel_err, max(fabsf( 0.5f * (data[memidx(i-1,0,shift)] + data[memidx(i+1,0,shift)]) - data[memidx(i,0,shift)] ) / (rel_err_eps + data[memidx(i,0,shift)])));

				rel_err = max(rel_err, max(fabsf( 0.5f * (data[memidx(0,i-1,shift)] + data[memidx(0,i+1,shift)]) - data[memidx(0,i,shift)] ) / (rel_err_eps + data[memidx(0,i,shift)])));

				rel_err = max(rel_err, max(fabsf( 0.5f * (data[memidx(i-1,cur_order-i+1,shift)] + data[memidx(i+1,cur_order-i-1,shift)]) - data[memidx(i,cur_order-i,shift)] ) / (rel_err_eps + data[memidx(i,cur_order-i,shift)])));

			}

	  		// inner
	  		// odd i
			for (uint i = 1; i < cur_order; i+=2){
				// odd j
				for (uint j = 1; i+j < cur_order; j+=2){
					const float u = float(i) * inv_cur_order;
					const float v = float(j) * inv_cur_order;
					data[memidx(i,j,shift)] = irr(prim, u, v);

					rel_err = max(rel_err, max(fabsf( 0.5f * (data[memidx(i-1,j+1,shift)] + data[memidx(i+1,j-1,shift)]) - data[memidx(i,j,shift)] ) / (rel_err_eps + data[memidx(i,j,shift)])));
				}

				// even j
				for (uint j = 2; i+j < cur_order; j+=2){
					const float u = float(i) * inv_cur_order;
					const float v = float(j) * inv_cur_order;
					data[memidx(i,j,shift)] = irr(prim, u, v);

					rel_err = max(rel_err, max(fabsf( 0.5f * (data[memidx(i,j-1,shift)] + data[memidx(i,j+1,shift)]) - data[memidx(i,j,shift)] ) / (rel_err_eps + data[memidx(i,j,shift)])));

				}
			}

	  		// even i
			for (uint i = 2; i < cur_order; i+=2){
				// odd j
				for (uint j = 1; i+j < cur_order; j+=2){
					const float u = float(i) * inv_cur_order;
					const float v = float(j) * inv_cur_order;
					data[memidx(i,j,shift)] = irr(prim, u, v);

					rel_err = max(rel_err, max(fabsf( 0.5f * (data[memidx(i-1,j,shift)] + data[memidx(i+1,j,shift)]) - data[memidx(i,j,shift)] ) / (rel_err_eps + data[memidx(i,j,shift)])));

				}
			}
		}

		const uint shift = max_exp - cur_exp;
		prim->irr_map.order = cur_order;
		prim->irr_map.data = (vecf4*)mem_alloc(size(cur_order) * sizeof(vecf4));

		uint idx = 0;
		for (uint r = 0; r <= cur_order; r++){
			for (uint j = 0; j <= r; j++){
				const uint i = r - j;
				prim->irr_map.data[idx] = data[memidx(i,j,shift)];
				idx++;
			}
		}
	}
}


} // irr_map