export default function Footer() {
  const currentYear = new Date().getFullYear();
  return (
    <div className="relative bg-[url('/footer.png')] bg-cover bg-top bg-no-repeat z-10 flex items-center justify-center py-5">
      <div className="mx-auto flex w-full max-w-7xl items-center justify-center gap-1.5">
        <p className="text-xs font-medium">
          {currentYear} - LoRa Access Control
        </p>
      </div>
    </div>
  );
}
