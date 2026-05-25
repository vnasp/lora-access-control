import { NextResponse } from "next/server";
import { supabase } from "@/lib/supabase";

const PARCEL_COUNT = 15;

function generateCode(): string {
  return String(Math.floor(100000 + Math.random() * 900000));
}

function todayUTC(): string {
  return new Date().toISOString().slice(0, 10);
}

export async function GET(request: Request) {
  const secret = request.headers.get("x-gateway-secret");
  if (secret !== process.env.GATEWAY_SECRET) {
    return NextResponse.json({ error: "Unauthorized" }, { status: 401 });
  }

  const date = todayUTC();

  // Fetch existing codes for today
  const { data: existing, error: fetchError } = await supabase
    .from("lora_daily_codes")
    .select("parcel_id, code")
    .eq("date", date)
    .order("parcel_id");

  if (fetchError) {
    return NextResponse.json({ error: fetchError.message }, { status: 500 });
  }

  // Generate missing codes
  const existingIds = new Set((existing ?? []).map((r) => r.parcel_id));
  const toInsert = [];
  for (let id = 1; id <= PARCEL_COUNT; id++) {
    if (!existingIds.has(id)) {
      toInsert.push({ date, parcel_id: id, code: generateCode() });
    }
  }

  if (toInsert.length > 0) {
    const { error: insertError } = await supabase
      .from("lora_daily_codes")
      .insert(toInsert);
    if (insertError) {
      return NextResponse.json({ error: insertError.message }, { status: 500 });
    }
  }

  // Return all codes for today ordered by parcel_id
  const allCodes = [
    ...(existing ?? []),
    ...toInsert.map((r) => ({ parcel_id: r.parcel_id, code: r.code })),
  ]
    .sort((a, b) => a.parcel_id - b.parcel_id)
    .map((r) => r.code);

  return NextResponse.json({ date, codes: allCodes });
}
