namespace irr_map{
// Funktion fuer die Messung der Irradianz
vecf4 irr(triangle* prim, float u, float v){
	const float w = 1.0f - u - v;

	// Bestimme Position im Dreieck
	pos =
		prim->v0.pos * w +
		prim->v1.pos * u +
		prim->v2.pos * v;

	// Bestimme Vertex-Shading-Normale
	n = prim->v0.n * w +
		prim->v1.n * u +
		prim->v2.n * v;

	vecf4::normalize(n);

	// Schaetze die Irradianz
	uint sample_count;
	return est_irr_adap(pos, n, sample_count);
}

// Irradiance Map Generator Funktion
void gen_irr_map(){
	const float rel_err_eps = 0.0001f; // Epsilon verhindert Division durch Null
	const float border_eps = 0.00001f; // Verschiebung der Randpunkte

	// Berechne die maximale Ordnung der Irradiance Map des Dreiecks aus einer Konstanten samples_per_unit
	const float samples_on_edge = samples_per_unit * prim->max_diag / mean_diag;
	const float cur_max_order = samples_on_edge + 1.0f;
	const uint cur_max_exp = max(ceilf(log2f(cur_max_order)), 1.0f);
	// Setze dennoch eine obere Schranke durch read_max_exp fuer die Berechnung.
	const uint max_exp = (cur_max_exp < read_max_exp)?(cur_max_exp):(read_max_exp);
	uint max_order = 1 << max_exp;

	// Berechne fuer die Konstruktion benoetigte Konstanten
	const uint half_max_order = max_order >> 1;
	const uint max_size = size(max_order);
	const uint half_max_size = size(half_max_order);
	// Speicherindizes der Eckpunkte
	const uint a = 0;
	const uint b = max_size-max_order-1;
	const uint c = max_size-1;
	// Speicherindizes der Kantenmittelpunkte
	const uint eab = half_max_size-half_max_order-1;
	const uint ebc = max_size-half_max_order-1;
	const uint eca = half_max_size-1;

	// Um bei der Generierung das Kopieren von Daten zu verhindern, verwende ein Array mit der maximalen Irradiance Map Groesse
	// Der Zugriff wird ueber die verfeinerte Speicherindex-Funktion moeglich sein
	vecf4 data[max_size];
	triangle* prim;

	// Fuehre die Generierung fuer alle Dreiecke durch. Diese Operation ist parallelisierbar.
	while(prim = next_triangle()){
		uint cur_exp = 1;
		uint cur_order = 1 << cur_exp;

		// Berechne Irradianz an Eckpunkten mit kleiner Verschiebung
		data[a] = irr(prim, border_eps, border_eps);
		data[c] = irr(prim, border_eps, 1.0f - 2.0f * border_eps);
		data[b] = irr(prim, 1.0f - 2.0f*border_eps, border_eps);

		// Berechne Irradianz an Mittelpunkt der Kanten mit kleiner Verschiebung (entspricht Approximation zweiter Ordnung)
		data[eca] = irr(prim, border_eps, 0.5f);
		data[eab] = irr(prim, 0.5f, border_eps);
		data[ebc] = irr(prim, 0.5f - border_eps, 0.5f - border_eps);

		// Berechne Fehlermass fuer zweite Ordnung
		float rel_err = max(fabsf( 0.5f*(data[a] + data[b]) - data[eab] ) / (rel_err_eps + data[eab]));
		rel_err = max(rel_err, max(fabsf( 0.5f*(data[b] + data[c]) - data[ebc] ) / (rel_err_eps + data[ebc])));
		rel_err = max(rel_err, max(fabsf( 0.5f*(data[c] + data[a]) - data[eca] ) / (rel_err_eps + data[eca])));

		// Verfeinere Approximation solange Fehlermass zu gross oder bis maximale Ordnung erreicht ist
		while (rel_err > rel_err_bound && cur_exp < max_exp){
			cur_exp++;
			cur_order = 1 << cur_exp;
			float inv_cur_order = 1.0f / float(cur_order);

	  		// Berechne Verfeinerungsgrad
			const uint shift = max_exp - cur_exp;

	  		// Berechne Irradianz und relativen Fehler fuer neue Messpunkte auf den Kanten mit kleiner Verschiebung
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

	  		// Berechne Irradianz und relativen Fehler fuer neue Messpunkte im Inneren des Dreiecks
			for (uint i = 1; i < cur_order; i+=2){ // ungerade i
				for (uint j = 1; i+j < cur_order; j+=2){ // ungerade j
					const float u = float(i) * inv_cur_order;
					const float v = float(j) * inv_cur_order;
					data[memidx(i,j,shift)] = irr(prim, u, v);

					rel_err = max(rel_err, max(fabsf( 0.5f * (data[memidx(i-1,j+1,shift)] + data[memidx(i+1,j-1,shift)]) - data[memidx(i,j,shift)] ) / (rel_err_eps + data[memidx(i,j,shift)])));
				}

				for (uint j = 2; i+j < cur_order; j+=2){ // gerade j
					const float u = float(i) * inv_cur_order;
					const float v = float(j) * inv_cur_order;
					data[memidx(i,j,shift)] = irr(prim, u, v);

					rel_err = max(rel_err, max(fabsf( 0.5f * (data[memidx(i,j-1,shift)] + data[memidx(i,j+1,shift)]) - data[memidx(i,j,shift)] ) / (rel_err_eps + data[memidx(i,j,shift)])));

				}
			}

			for (uint i = 2; i < cur_order; i+=2){ // gerade i
				for (uint j = 1; i+j < cur_order; j+=2){ // ungerade j
					const float u = float(i) * inv_cur_order;
					const float v = float(j) * inv_cur_order;
					data[memidx(i,j,shift)] = irr(prim, u, v);

					rel_err = max(rel_err, max(fabsf( 0.5f * (data[memidx(i-1,j,shift)] + data[memidx(i+1,j,shift)]) - data[memidx(i,j,shift)] ) / (rel_err_eps + data[memidx(i,j,shift)])));

				}
			}
		}

		// Alloziere durch eine optimierte Funktion Speicher fuer die Irradiance Map des Dreiecks
		const uint shift = max_exp - cur_exp;
		prim->irr_map.order = cur_order;
		prim->irr_map.data = (vecf4*)mem_alloc(size(cur_order) * sizeof(vecf4));

		// Kopiere die temporaeren Daten im Array in die Irradiance Map
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