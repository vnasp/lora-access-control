import { supabase } from "@/lib/supabase";

function normalizeRut(rut: string) {
  return rut.replace(/[.\s]/g, "").toLowerCase().trim();
}

export async function POST(req: Request) {
  const { parcel_id, rut } = await req.json();

  if (!parcel_id || !rut) {
    return Response.json({ ok: false, error: "Datos incompletos" }, { status: 400 });
  }

  const { data, error } = await supabase
    .from("lora_parcels")
    .select("parcel_id")
    .eq("parcel_id", Number(parcel_id))
    .eq("rut", normalizeRut(rut))
    .single();

  if (error) console.error("[auth]", error.code, error.message);

  if (!data) {
    return Response.json({ ok: false, error: "Parcela o RUT incorrecto" }, { status: 401 });
  }

  return Response.json({ ok: true });
}
