"use client";

import { useRouter } from "next/navigation";
import { RefreshCw } from "lucide-react";
import { useState } from "react";

export default function RefreshButton() {
  const router = useRouter();
  const [spinning, setSpinning] = useState(false);

  function handleRefresh() {
    setSpinning(true);
    router.refresh();
    setTimeout(() => setSpinning(false), 800);
  }

  return (
    <button onClick={handleRefresh} className="rounded  p-1.5" title="Recargar">
      <RefreshCw
        className={`h-6 w-6  transition-transform ${spinning ? "animate-spin" : ""}`}
      />
    </button>
  );
}
